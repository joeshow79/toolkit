from PIL import Image
import numpy as np
import os

IMG_PATH = "./benchmark/PNGImages"
MASK_PATH = "./benchmark/Masks"
ANNOTATION_PATH = "./benchmark/Annotation"

def make_annotation(mask_path):
    #im = Image.open('mask2.png')
    im = Image.open(mask_path)
    masks = np.asarray(im)

    rects = []

    for i in range(1):

        #  Create figure and axes
        # fig,ax = plt.subplots(1)

        mask = masks

        # Display the image
        # ax.imshow(mask,cmap=plt.cm.gray)

        subregion_x_start = 0
        subregion_x_end = 0

        search_mask_or_non_mask = True

        found_sub_region = False

        for i in range(0, mask.shape[1]-1):
            # print("processing column:{}".format(i))
            counter = 0
            for j in range(0, mask.shape[0]-1):
                if mask[j, i] > 0:
                    # print("mask value:{}".format(mask[j,i]))
                    if subregion_x_start < subregion_x_end:
                        print("Move x start pos.")
                        search_mask_or_non_mask = True
                        subregion_x_start = i

                    break
                else:
                    counter = counter + 1

            # print("ymax:{}, ymin:{}, ymax-ymin:{}, counter:{}".format(ymax, ymin, ymax-ymin, counter))

            if counter >= mask.shape[0]-1:
                if search_mask_or_non_mask:
                    subregion_x_end = i

                    sub_coor = np.nonzero(mask[:, subregion_x_start:subregion_x_end])

                    if len(sub_coor[0]) == 0:
                        continue

                    print("sub region found x_start:{}, x_end:{}".format(subregion_x_start, subregion_x_end))

                    # print("-->{}".format(sub_coor[0][0]))

                    sub_ymin = sub_coor[0][0]
                    sub_ymax = sub_coor[0][-1]
                    sub_coor[1].sort() # 直接改变原数组，没有返回值
                    sub_xmin = sub_coor[1][0] + subregion_x_start
                    sub_xmax = sub_coor[1][-1] + subregion_x_start

                    sub_bottomleft = (sub_xmin, sub_ymin)

                    sub_width = sub_xmax - sub_xmin
                    sub_height = sub_ymax - sub_ymin

                    # rect = patches.Rectangle(sub_bottomleft,sub_width,sub_height,linewidth=1,edgecolor='b',facecolor='none')
                    # ax.add_patch(rect)

                    rect = (sub_bottomleft, sub_width, sub_height)
                    #rects = [rects, rect]
                    rects.extend([rect])


                    search_mask_or_non_mask = False

    # wirte Annotation
    ##  Begin of Annotation example
    ## Compatible with PASCAL Annotation Version 1.00
    #Image filename : "PennFudanPed/PNGImages/FudanPed00001.png"
    #Image size (X x Y x C) : 559 x 536 x 3
    #Database : "The Penn-Fudan-Pedestrian Database"
    #Objects with ground truth : 2 { "PASpersonWalking" "PASpersonWalking" }
    ## Note there may be some objects not included in the ground truth list for they are severe-occluded
    ## or have very small size.
    ## Top left pixel co-ordinates : (1, 1)
    ## Details for pedestrian 1 ("PASpersonWalking")
    #Original label for object 1 "PASpersonWalking" : "PennFudanPed"
    #Bounding box for object 1 "PASpersonWalking" (Xmin, Ymin) - (Xmax, Ymax) : (160, 182) - (302, 431)
    #Pixel mask for object 1 "PASpersonWalking" : "PennFudanPed/PedMasks/FudanPed00001_mask.png"

    ## Details for pedestrian 2 ("PASpersonWalking")
    #Original label for object 2 "PASpersonWalking" : "PennFudanPed"
    #Bounding box for object 2 "PASpersonWalking" (Xmin, Ymin) - (Xmax, Ymax) : (420, 171) - (535, 486)
    #Pixel mask for object 2 "PASpersonWalking" : "PennFudanPed/PedMasks/FudanPed00001_mask.png"
    ##End of example

    base_filename = os.path.splitext(os.path.splitext(os.path.basename(mask_path))[0])[0]
    img_filename = base_filename + ".png"
    mask_filename = base_filename + ".mask.png"
    annot_filename = base_filename + ".txt"
    annot_filepath = os.path.join(ANNOTATION_PATH, annot_filename)
    print(annot_filepath)

    with open(annot_filepath, 'wt') as f:
        print("# Compatible with PASCAL Annotation Version 1.00", file=f)
        print("Image filename : \"benchmark/PNGImages/{}".format(img_filename), file=f)
        print("Image size (X x Y x C) : 672 x 512 x 1", file=f)
        print("Database : \"The JiaSen-InstantNoodleBox Database\"", file=f)

        print("Objects with ground truth : {} {{ ".format(len(rects)), file=f, end='')
        for rect in rects:
            print(" \"PackageDefect\"", file=f, end='')
        print(" }}".format(len(rects)), file=f)

        print("# Note there may be some objects not included in the ground truth list for they are severe-occluded", file=f)
        print("# or have very small size.", file=f)
        print("# Top left pixel co-ordinates : (1, 1)", file=f)

        index = 0
        for rect in rects:
            print("# Details for defect {} (\"PackageDefect\")".format(index+1), file=f)
            print("Original label for object {} \"PackageDefect\" : \"benchmark\"".format(index+1), file=f)
            print("Bounding box for object {} \"PackageDefect\" (Xmin, Ymin) - (Xmax, Ymax) : ({}, {}) - ({}, {})".format(index, \
                    rects[index][0][0], rects[index][0][1], \
                    rects[index][0][0] + rects[index][1], \
                    rects[index][0][1] + rects[index][2]), file=f)
            print("Pixel mask for object {} \"PackageDefect\" : \"benchmark/Masks/{}\"".format(index+1, mask_filename), file=f)

            index = index + 1


#         plt.show()

if __name__ == '__main__':
    for root,dirs,files in os.walk(MASK_PATH):
        for file in files:
            print("process {}".format(os.path.join(root, file)))
            make_annotation(os.path.join(root, file))

    #make_annotation("benchmark/PedMasks/1582254522665434.mask.png")
