import numpy as np
import cv2 
import matplotlib.pyplot as plt

IMAGE_FILE = './data/chessboard_lightfield.png'

#Init
im = cv2.imread(IMAGE_FILE)

U_SCALE = 16 #Vertical/Height
V_SCALE = 16 #Horizontal/Width

#Sub-aperture view
subapers = np.array([[im[i::U_SCALE,j::V_SCALE] for i in range(U_SCALE)] for j in range(V_SCALE)])
print("subapers shape:",subapers.shape)

col= len(subapers)
row = len(subapers[0])
print("col:",col," row:",row)

plt.figure(figsize=(24,18))
fig,axes = plt.subplots(nrows=row,ncols=col,sharex=True,sharey=True)

for i in range(row):
    for j in range(col):
        axes[i,j].axis('off')
        axes[i,j].imshow(subapers[i][j][:,:,::-1])
        #cv2.imwrite(str(i)+"_"+str(j)+".png",subapers[i][j])

plt.savefig('./sub_apertures_view.png')

#Focal stack generation
im_list = np.array([j for img in subapers for j in img])
#print(im1.shape)

from scipy import interpolate

L_SCALE = 16 #lenslet
def interp(im_subaperture,u,v):
    h,w,c = im_subaperture.shape
    x,y = np.arange(w),np.arange(h)

    func = [interpolate.interp2d(x,y,im_subaperture[:,:,i],kind='linear') for i in range(c)]
    def wrapper(d):
        #x1,y1 = x - d * (v - L_SCALE/2), y + d * (u - L_SCALE/2)
        x1,y1 = x - d * (u - L_SCALE/2), y + d * (v - L_SCALE/2)
        return np.array([f(x1,y1) for f in func]).transpose((1,2,0))
    return wrapper


def refocus(d):
    im_refocus = [interp(subapers[i][j],i,j)(d) for i in range(col) for j in range(row)]
    return np.array(im_refocus).mean(0).astype('uint8')

ds = [0,0.1,0.3,0.5,0.7,0.9,1.1,1.3]
focal_stack = [refocus(d) for d in ds]

for i,f in enumerate(focal_stack):
    cv2.imwrite(str(ds[i])+".png",f)

#Drawing focal stack
num = len(focal_stack)
row = (num+1)/2

plt.figure(figsize=(24,18))

for i in range(num):
    plt.subplot(row,2,i+1)
    plt.imshow(focal_stack[i][:,:,::-1])
    plt.axis('off')
    plt.title(str('focal stack '+str(ds[i])))

plt.savefig('./focal_stack.png')

#All in focus
from skimage import color

sigma_1,sigma_2 = 11,11
h,w,c = focal_stack[0].shape

#Color conversion
im_luminance = [color.rgb2xyz(img[:,:,::-1])[:,:,1] for img in focal_stack]


#Low freq.
im_lowfreq = [cv2.GaussianBlur(img,(sigma_1,sigma_1),0) for img in im_luminance]

#High freq.
im_highfreq = [im_luminance[i] - im_lowfreq[i] for i in range(len(im_lowfreq))]

#Sharpness weight
w_sharpness = [cv2.GaussianBlur(img**2,(sigma_2,sigma_2),0) for img in im_highfreq]

w_sharpness_3 = [np.tile(img,(c,1)).reshape((c,h,w)).transpose((1,2,0)) for img in w_sharpness]

#All in focus
im_allinfocus = np.array([w_sharpness_3[i] * focal_stack[i] for i in range(len(w_sharpness_3))]).sum(0)/np.array(w_sharpness_3).sum(0)

im_allinfocus = np.uint8(im_allinfocus[:,:,::-1])

plt.figure(figsize=(24,18))
plt.imshow(im_allinfocus)
plt.title('all_in_focus image')
plt.imsave('./all_in_focus.png',im_allinfocus)

im_depth_1 = np.array([w_sharpness_3[i] * ds[i] for i in range(len(w_sharpness_3))]).sum(0)
im_depth_2 = np.array(w_sharpness_3).sum(0)
im_depth = im_depth_1 / im_depth_2
im_depth = im_depth[:,:,::-1].clip(0,1)

plt.figure(figsize=(24,18))
plt.imshow(im_depth)
plt.title('depth map')
plt.imsave('./depth.png',im_depth)


