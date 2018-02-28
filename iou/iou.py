import sys
import os
from scipy.misc import imread
import numpy as np


def calcIoU(img1,img2):
    mask=imread(img1)
    mask=mask/255
    maskGT=imread(img2)
    maskGT=maskGT/255

    intersection=np.multiply(mask,maskGT)
    union=np.add(mask,maskGT)

    print ">>Processing:" + img1
    print "I(pixel):"
    #print "Shape:"+str(intersection.shape)
    i=np.sum(intersection[intersection>0])
    print i
    print "U(pixel):"
    #print "Shape:"+str(union.shape)
    u=np.sum(union[union>0])
    s=np.sum(union[union>1]) # for the 1 + 1 = 2 case
    u=u-s/2
    #print s
    print u
    print "IoU:"
    print float(i)/u
    return float(i)/u


if __name__== '__main__':
    print "Usage:"
    print "python iou.py mask_dir groundtruth_dir"
    print ""
    maskDir=sys.argv[1]
    maskGTDir=sys.argv[2]
    count=0
    count80=0
    count90=0
    count95=0
    count100=0
    sumIoU=0.0
    for (root,dirs,files) in os.walk(maskDir):
        for filename in files:
            if os.path.exists(os.path.join(maskGTDir,filename)):
                iou = calcIoU(os.path.join(root,filename),os.path.join(maskGTDir,filename))
                
                sumIoU = sumIoU + iou
                count = count + 1
                if iou < 0.8 :
                    count80 = count80 + 1
                else:
                    if iou < 0.9 :
                        count90 = count90 + 1
                    else:
                        if iou < 0.95 :
                            count95 = count95 + 1
                        else:
                            count100 = count100 +1
            else:
                print "!Skip image "+os.path.join(root,filename)+" due to no GT matched"


    
    print "=============================================="
    print "Summary:"
    print "IoU Range:[0,0.8) [0.8,0.9) [0.9,0.95) [0.95,1.0]"
    print "    Count:\t"+str(count80)+"\t"+str(count90)+"\t"+str(count95)+"\t"+str(count100)
    print "----------------------------------------------"
    print "Total # of processed images:" + str(count)
    print "Average IoU:"
    print float(sumIoU)/count

