import torch
import argparse
from INBDataset import INBDataset
import model as M
from engine import evaluate
import utils


DEBUG = False


def print_if_debug(msg):
    if DEBUG:
        print(msg)


def stat(model, dataset, device):
    data_loader_test = torch.utils.data.DataLoader(
        dataset_test, batch_size=1, shuffle=False, num_workers=2,
        collate_fn=utils.collate_fn)

    # move model to the right device
    model.to(device)

    evaluate(model, data_loader_test, device=device)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-b", "--backbone", type=str,
                        choices=['resnet', 'mn2'], default='resnet',
                        help="backbone type")
    parser.add_argument("-m", "--modelpath",
                        default='./checkpoint/inb_resnet_9',
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

    stat(model, dataset_test, device)
