import os
import time
import numpy as np
import sys
import cv2
import time


caffe_root="/Users/jasonjmac/src/ENet/caffe-enet/"
sys.path.insert(0,caffe_root+"python")

import caffe

enet_weight="/Users/jasonjmac/src/toolkit/rtbokeh/stuart_enet_150000_multiscale_agumentation_rotate_resize_20171120_120x160_100000.caffemodel"
enet_model="/Users/jasonjmac/src/toolkit/rtbokeh/stuart_deploy_final.prototxt"

background_img="./timg.jpg"


def imresize(im,sz):
    pil_im=Image.fromarray(uint8(im))

    return array(pil_im.resize(sz))


if __name__ == '__main__':
    net=caffe.Net(enet_model,enet_weight,caffe.TEST)

    input_shape=net.blobs['data'].data.shape
    output_shape=net.blobs['deconv6_0_0_jj'].data.shape

    video=cv2.VideoCapture(0)

    background=cv2.imread(background_img)
    background=cv2.resize(background,(1280,720))

    while True:
        t_base=time.time() 
        ret,orig_input_image=video.read()
        orig_input_shape=orig_input_image.shape
        if ret == True:
            input_image=cv2.resize(orig_input_image,(input_shape[3],input_shape[2]))
            input_image=input_image.transpose((2,0,1))
            input_image=np.asarray([input_image])

            inference_time_base=time.time() 
            out=net.forward_all(**{net.inputs[0]:input_image})
            print ("Inference time: "+str((time.time()-inference_time_base)*1000))

            prediction=net.blobs['deconv6_0_0_jj'].data[0].argmax(axis=0)
            prediction=np.squeeze(prediction).astype(np.uint8)

            prediction=255 * prediction

            prediction=cv2.resize(prediction,(orig_input_shape[1],orig_input_shape[0]))

            #print orig_input_image.shape
            #print prediction.shape

            #print type(orig_input_image)

            output_image=orig_input_image.copy()
            output_image[:,:,0]=prediction[:,:]
            output_image[:,:,1]=prediction[:,:]
            output_image[:,:,2]=prediction[:,:]

            output_image=cv2.addWeighted(orig_input_image,0.5,output_image,0.5,0.0)
            #output_image=cv2.addWeighted(background,0.5,output_image,0.5,0.0)
            #output_image.copyTo(background,prediction)

            cv2.imshow('rtbokeh',output_image)
            print ("1 iteration time: "+str((time.time()-t_base)*1000))

        if cv2.waitKey(10) == 27:
            break
