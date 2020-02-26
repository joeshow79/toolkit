import torch
import torchvision
from torchvision.models.detection.faster_rcnn import FastRCNNPredictor
from torchvision.models.detection.mask_rcnn import MaskRCNNPredictor
import numpy as np
from PIL import Image, ImageDraw
import transforms as T
from INBOkDataset import INBOkDataset
import time

DEBUG = False


def print_if_debug(msg):
    if DEBUG:
        print(msg)


def show_sample(img, target, show=True):
    """
    Show a sample image
    :param img: Tensor of shape (3, H, W)
    :param target: target dictionary. Should have 'boxes' as key
    :param show: (boolean) if True show the image
    :return: PIL image of the sample with bounding boxes
    """
    img_arr = img.numpy()
    img_arr = img_arr.transpose((1, 2, 0)) * 255
    img_arr = img_arr.astype(np.uint8)

    image = Image.fromarray(img_arr)
    bboxes = target['boxes'].numpy()
    bboxes = np.ceil(bboxes)

    draw = ImageDraw.Draw(image)
    for bbox_id in range(bboxes.shape[0]):
        bbox = list(bboxes[bbox_id, :])
        draw.rectangle(bbox, outline=(255, 0, 255))

    if show:
        image.save('test.png')

    return image


def show_sample_and_pred(img, target, pred, masks, output_file=None):
    """
    Show a sample image
    :param img: Tensor of shape (3, H, W)
    :param target: target dictionary. Should have 'boxes' as key
    :param pred: pred result after inference
    :param output_file: (boolean) if True store the image
    :return: PIL image of the sample with bounding boxes
    """
    img_arr = img.numpy()
    img_arr = img_arr.transpose((1, 2, 0)) * 255
    img_arr = img_arr.astype(np.uint8)

    image = Image.fromarray(img_arr)
    bboxes = target['boxes'].numpy()
    bboxes = np.ceil(bboxes)

    draw = ImageDraw.Draw(image)
    for bbox_id in range(bboxes.shape[0]):
        bbox = list(bboxes[bbox_id, :])
        print_if_debug('bbox:{}'.format(bbox))
        draw.rectangle(bbox, outline=(255, 0, 255))

    print_if_debug('bbox in pred:{}'.format(pred[0]['boxes']))
    for bbox in pred[0]['boxes']:
        print_if_debug('bbox:{}'.format(bbox))
        draw.rectangle(bbox.detach().numpy(), outline=(255, 255, 255))

    # Mask
    # for row in range(img_arr.shape[0]):
    #     for col in range(img_arr.shape[1]):
    #         if masks[row, col] > 0:
    #              draw.point((col, row))

    if output_file is not None:
        image.save(output_file + ".png")

        for i in range(0, masks.shape[0]):  # go through all masks
            mask = masks[i]
            mask = mask.transpose(1, 2, 0).astype("uint8")
            mask = mask[:, :, 0]
            mask_img = Image.fromarray(mask)
            mask_img.save(output_file + "_mask_" + str(i) + ".png")

    return image


def get_model_instance_segmentation(num_classes):
    # load an instance segmentation model pre-trained pre-trained on COCO
    model = torchvision.models.detection.maskrcnn_resnet50_fpn(pretrained=True)

    # get number of input features for the classifier
    in_features = model.roi_heads.box_predictor.cls_score.in_features
    # replace the pre-trained head with a new one
    model.roi_heads.box_predictor = FastRCNNPredictor(in_features, num_classes)

    # now get the number of input features for the mask classifier
    in_features_mask = model.roi_heads.mask_predictor.conv5_mask.in_channels
    hidden_layer = 256
    # and replace the mask predictor with a new one
    model.roi_heads.mask_predictor = MaskRCNNPredictor(in_features_mask,
                                                       hidden_layer,
                                                       num_classes)

    return model


def get_transform(train):
    transforms = []
    transforms.append(T.ToTensor())
    if train:
        transforms.append(T.RandomHorizontalFlip(0.5))
    return T.Compose(transforms)


def inference(model, dataset, device, data_index=1, img_path=None):
    if data_index is not None:
        input_img, target = dataset[data_index]

    cpu_device = torch.device("cpu")

    model.eval()
    batch_input_img = input_img.unsqueeze(0)
    batch_input_img = input_img.to(device)
    batch_input_img = [batch_input_img]

    # with torch.no_grad():
    # model.to(device)
    # pred = model(batch_input_img)
    # print_if_debug("type:{}".format(type(pred[0])))
    # print_if_debug(pred[0])

    torch.cuda.synchronize()
    model_time = time.time()
    outputs = model(batch_input_img)

    outputs = [{k: v.to(cpu_device) for k, v in t.items()} for t in outputs]
    model_time = time.time() - model_time

    # Debug only begin
    mask_0 = outputs[0]['masks']
    print_if_debug('mask shape 1:{}'.format(mask_0.shape))
    print_if_debug('mask_0:{}'.format(mask_0))

    mask_0 = mask_0 * 255
    mask_0.clamp(0, 255) % 255
    masks_np = mask_0.detach().numpy()

    print_if_debug('masks:{}'.format(masks_np))
    print_if_debug('scores:{}'.format(outputs[0]['scores']))
    print_if_debug('boxes:{}'.format(outputs[0]['boxes']))
    print_if_debug('labels:{}'.format(outputs[0]['labels']))
    # Debug only end

    show_sample_and_pred(input_img, target, outputs, masks_np,
                         'out/'+str(data_index))


if __name__ == '__main__':
    dataset = INBOkDataset('benchmark', get_transform(train=False))
    indices = torch.randperm(len(dataset)).tolist()
    dataset_test = torch.utils.data.Subset(dataset, indices[-50:])  # FIXME

    model = get_model_instance_segmentation(2)
    checkpoint = torch.load('./checkpoint/inb_epoch_9')
    model.load_state_dict(checkpoint['model_state_dict'])
    device = torch.device('cuda') if torch.cuda.is_available()\
        else torch.device('cpu')
    model.to(device)

    for image_index in range(0, 50):
        inference(model, dataset_test, device, image_index)
