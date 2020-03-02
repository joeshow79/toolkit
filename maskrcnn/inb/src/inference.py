import time
import numpy as np
import torch
from PIL import Image, ImageDraw
import argparse
from INBDataset import INBDataset
import model as M
from skimage import measure
import matplotlib.pyplot as plt


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


def show_sample_and_pred(img, target, pred, masks,
                         output_file=None, filtered_boxes_id=None):
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

    fig, ax = plt.subplots()
    ax.set_xticks([])
    ax.set_yticks([])
    ax.imshow(img_arr, cmap=plt.cm.gray)

    # GT BBox
    draw = ImageDraw.Draw(image)
    for bbox_id in range(bboxes.shape[0]):
        bbox = list(bboxes[bbox_id, :])
        print_if_debug('bbox:{}'.format(bbox))
        draw.rectangle(bbox, outline=(255, 0, 255))
        rect = plt.Rectangle((bbox[0], bbox[1]),
                             bbox[2]-bbox[0], bbox[3]-bbox[1],
                             fill=False, edgecolor='red', linewidth=1)
        ax.add_patch(rect)

    # Predicted BBox
    print_if_debug('bbox in pred:{}'.format(pred[0]['boxes']))
    index = 0

    result = pred[0]
    for bbox in result['boxes']:
        if (filtered_boxes_id is not None) and (index in filtered_boxes_id):
            continue
        else:
            print_if_debug('bbox:{}'.format(bbox))
            draw.rectangle(bbox.numpy(), outline=(255, 255, 255))
            rect = plt.Rectangle((bbox[0], bbox[1]),
                                 bbox[2]-bbox[0], bbox[3]-bbox[1],
                                 fill=False, edgecolor='white', linewidth=1)
            ax.add_patch(rect)
            ax.text(bbox[0], bbox[1], 'class:{} conf:{:.2f}'.format(
                result['labels'][index].numpy(),
                result['scores'][index].numpy()),
                bbox={'facecolor': 'white', 'alpha': 0.5, 'pad': 0})

        index = index + 1

    if output_file is not None:
        for i in range(0, len(masks)):  # go through all masks
            mask = masks[i]
            contours = measure.find_contours(mask[0], 0.8)

            # ROI contour
            for n, contour in enumerate(contours):
                ax.plot(contour[:, 1], contour[:, 0],
                        linewidth=1, color='orange')

            mask = mask.transpose(1, 2, 0).astype("uint8")
            mask = mask[:, :, 0]
            mask_img = Image.fromarray(mask)
            mask_img.save(output_file + "_mask_" + str(i) + ".png")

        # image.save(output_file + ".jpg")
        plt.savefig(output_file + ".png")

    plt.close()

    return image


def inference(model, dataset, device, data_index=1, img_path=None):
    if data_index is not None:
        input_img, target = dataset[data_index]

    cpu_device = torch.device("cpu")

    model.eval()
    batch_input_img = input_img.unsqueeze(0)
    batch_input_img = input_img.to(device)
    batch_input_img = [batch_input_img]

    with torch.no_grad():
        torch.cuda.synchronize()
        model_time = time.time()
        outputs = model(batch_input_img)

        outputs = [{k: v.to(cpu_device) for k, v in t.items()}
                   for t in outputs]
        model_time = time.time() - model_time

        # Debug only begin
        mask_0 = outputs[0]['masks']
        print_if_debug('mask shape 1:{}'.format(mask_0.shape))
        print_if_debug('mask_0:{}'.format(mask_0))

        mask_0 = mask_0 * 255
        mask_0.clamp(0, 255) % 255
        masks_np = mask_0.numpy()

        print_if_debug('masks:{}'.format(masks_np))
        print_if_debug('scores:{}'.format(outputs[0]['scores']))
        print_if_debug('boxes:{}'.format(outputs[0]['boxes']))
        print_if_debug('labels:{}'.format(outputs[0]['labels']))
        # Debug only end

        # Mask process 1: Filter out mask of small area or low confidence
        filtered_masks = []
        filtered_boxes_id = []
        index = 0
        for mask_np in masks_np:
            # TODO: Threshold the mask
            mask_np = (mask_np > 128) * mask_np

            sub_nz = np.nonzero(mask_np)
            if len(mask_np[sub_nz]) >= 100:  # FIXME  skip < 10x10 patch
                filtered_masks.append(mask_np)
            else:
                filtered_boxes_id.append(index)

            index += 1

        # Mask process 2: Merge
        if len(filtered_masks) > 0:
            filtered_masks = np.mean(filtered_masks, 0).astype(int)
            filtered_masks = [filtered_masks]

        show_sample_and_pred(input_img, target, outputs, filtered_masks,
                             'out/' + '{:0>3d}'.format(data_index),
                             filtered_boxes_id)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-b", "--backbone", type=str,
                        choices=['resnet', 'mn2'], default='mn2',
                        help="backbone type")
    parser.add_argument("-m", "--modelpath",
                        default='./checkpoint/inb_mobilenetv2_epoch_9',
                        help="model file path")
    parser.add_argument("-d", "--datapath", default='./benchmark',
                        help="data path")
    args = parser.parse_args()

    dataset = INBDataset(args.datapath, M.get_transform(train=False))

    dataset_test = dataset

    num_classes = 2

    model = M.get_model_instance_segmentation(args.backbone, num_classes)

    checkpoint = torch.load(args.modelpath)
    # model.load_state_dict(checkpoint['model_state_dict'])
    model.load_state_dict(checkpoint)

    device = torch.device('cuda') if torch.cuda.is_available()\
        else torch.device('cpu')
    model.to(device)

    for image_index in range(0, 100):  # FIXME
        inference(model, dataset_test, device, image_index)
