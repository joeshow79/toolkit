import argparse
import model as M


DEBUG = False


def print_if_debug(msg):
    if DEBUG:
        print(msg)


if __name__ == '__main__':
    parser = argparse.ArgumentParser("utility for inspecting the model.")
    parser.add_argument("-b", "--backbone", type=str,
                        choices=['resnet', 'mn2'], default='resnet',
                        help="backbone type")
    args = parser.parse_args()

    num_classes = 2

    model = M.get_model_instance_segmentation(args.backbone, num_classes)

    total_params = 0
    layer = 0
    for p in model.parameters():
        num = format(p.numel(), ',')
        print('Layer:{} Params Num:{}'.format(layer, num))
        total_params += p.numel()
        layer += 1

    # pytorch_total_params = sum(p.numel() for p in model.parameters()
    # if p.requires_grad)

    print('Total params:{}'.format(format(total_params, ',')))
