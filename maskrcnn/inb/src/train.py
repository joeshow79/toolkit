import os
import utils
import argparse
import torch
from engine import train_one_epoch, evaluate
from INBDataset import INBDataset
import model as M


def train(model, dataset, dataset_test, device, prefix):
    # split the dataset in train and test set
    indices = torch.randperm(len(dataset)).tolist()
    dataset = torch.utils.data.Subset(dataset, indices[:-50])
    dataset_test = torch.utils.data.Subset(dataset_test, indices[-50:])

    # define training and validation data loaders
    data_loader = torch.utils.data.DataLoader(
        dataset, batch_size=2, shuffle=True, num_workers=2,
        collate_fn=utils.collate_fn)

    data_loader_test = torch.utils.data.DataLoader(
        dataset_test, batch_size=1, shuffle=False, num_workers=2,
        collate_fn=utils.collate_fn)

    # move model to the right device
    model.to(device)

    # construct an optimizer
    params = [p for p in model.parameters() if p.requires_grad]
    optimizer = torch.optim.SGD(params, lr=0.005,
                                momentum=0.9, weight_decay=0.0005)
    # and a learning rate scheduler
    lr_scheduler = torch.optim.lr_scheduler.StepLR(optimizer,
                                                   step_size=3,
                                                   gamma=0.1)

    # let's train it for 10 epochs
    num_epochs = 10

    for epoch in range(num_epochs):
        # train for one epoch, printing every 10 iterations
        train_one_epoch(model, optimizer, data_loader,
                        device, epoch, print_freq=10)

        # torch.save({'epoch': epoch, 'model_state_dict': model.state_dict(),
        # 'optimizer_state_dict': optimizer.state_dict(), },
        # os.path.join("checkpoint/", prefix + "_" + str(epoch)))

        torch.save(model.state_dict(),
                   os.path.join("checkpoint/", prefix + "_" + str(epoch)))

        # update the learning rate
        lr_scheduler.step()
        # evaluate on the test dataset
        evaluate(model, data_loader_test, device=device)

    print("That's it!")


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-b", "--backbone", type=str,
                        choices=['resnet', 'mn2'], default='mn2',
                        help="backbone type")
    parser.add_argument("-p", "--prefix",
                        default='inb_mn2_epoch',
                        help="model file prefix")
    parser.add_argument("-d", "--datapath", default='./data',
                        help="data path")
    args = parser.parse_args()

    # train on the GPU or on the CPU, if a GPU is not available
    device = torch.device('cuda') if torch.cuda.is_available()\
        else torch.device('cpu')

    # use our dataset and defined transformations
    dataset = INBDataset('data', M.get_transform(train=True))
    dataset_test = INBDataset('data', M.get_transform(train=False))

    # our dataset has two classes only -
    num_classes = 2

    # get the model using our helper function
    model = M.get_model_instance_segmentation(args.backbone, num_classes)

    train(model, dataset, dataset_test, device, args.prefix)
