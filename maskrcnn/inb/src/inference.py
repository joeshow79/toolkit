import torch
import torchvision
from torchvision.models.detection.faster_rcnn import FastRCNNPredictor
from torchvision.models.detection.mask_rcnn import MaskRCNNPredictor
import numpy as np
from PIL import Image, ImageDraw
import transforms as T
from INBDataset import INBDataset

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


def show_sample_and_pred(img, target, pred, output_file=None):
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
        draw.rectangle(bbox.numpy(), outline=(255, 255, 255))

    if output_file is not None:
        image.save(output_file)

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


def inference(dataset, data_index=1, img_path=None):
    # device = torch.device("cuda")
    device = torch.device("cpu")

    if data_index is not None:
        input_img, target = dataset[data_index]
    # show_sample(input_img, target)

    # print_if_debug("debug:{}".format(input_img))

    batch_input_img = input_img.unsqueeze(0)
    # debug_info = batch_input_img.numpy()
    # print_if_debug("debug numpy shape:{}".format(debug_info.shape))

    model = get_model_instance_segmentation(2)

    checkpoint = torch.load('./checkpoint/inb_epoch_9')
    model.load_state_dict(checkpoint['model_state_dict'])
    model.eval()
    # transform = get_transform(False)
    # transform = T.Compose([ T.ToTensor() ])

    # img = Image.open(img_path)
    # img = img.unsqueeze(0).to(device)
    # img = img.to(device)

    # img = utils.load_image(img_path)
    # img = transform(img, None)
    # print_if_debug('image:{}'.format(img))
    # print_if_debug('image shape 1:{}'.format(img[0].numpy().shape))

    with torch.no_grad():
        # model.to(device)
        pred = model(batch_input_img)
        print_if_debug("type:{}".format(type(pred[0])))
        print_if_debug(pred[0])
    # output = pred[0]['masks'].numpy().squeeze()
    mask = pred[0]['masks']
    print_if_debug('mask shape 1:{}'.format(mask.shape))
    print_if_debug('masks:{}'.format(mask))
    # mask = mask.detach().clamp(0,255).numpy()
    mask = mask.detach().numpy()
    print_if_debug('mask shape 2:{}'.format(mask.shape))

    mask = mask[0]
    mask = mask.transpose(1, 2, 0).astype("uint8")
    mask = mask[:, :, 0]
    img = Image.fromarray(mask)
    img.save('mask.png')
    # np.savetxt('test.txt', img, fmt='%d')
    np.savetxt('test.txt', img, fmt='%1.4e')

    print_if_debug('masks:{}'.format(mask))
    print_if_debug('scores:{}'.format(pred[0]['scores']))
    print_if_debug('boxes:{}'.format(pred[0]['boxes']))
    print_if_debug('labels:{}'.format(pred[0]['labels']))

    show_sample_and_pred(input_img, target, pred,
                         'out/'+str(data_index)+'.png')


if __name__ == '__main__':
    dataset = INBDataset('data', get_transform(train=False))
    indices = torch.randperm(len(dataset)).tolist()
    dataset_test = torch.utils.data.Subset(dataset, indices[-50:])

    for image_index in range(0, 1):
        inference(dataset_test, image_index)
