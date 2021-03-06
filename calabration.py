import numpy as np
import cv2 
from IPython import embed
from freenect import sync_get_depth as get_depth, sync_get_video as get_video
import matplotlib.pyplot as plt 

class Calabration(object):
    
    def __init__(self,camera,sandbox_depth, no_points=10, projector_resolution=(878 , 1440)):
        self.sandbox_depth = sandbox_depth
        self.no_points     = no_points
        self.projector_resolution = projector_resolution
        
    def generate_chessboard(self,center,segments,size):
        board = np.zeros((size,size,3),np.uint8)
        seg_size = size/segments
        for x in np.arange(size):
            for y in np.arange(size):
                if (x/seg_size + (y/seg_size +1))%2==0:
                    board[x,y]= (255,255,255)
        # cv2.imshow('board', board)    
        img = np.zeros((self.projector_resolution[0], self.projector_resolution[1] ,3), np.uint8)
        img[:,:,1]=255
        img[center[0]-size/2:center[0]+size/2, center[1]-size/2:center[1]+size/2, :] =  board
        corners=[]
        for x in np.arange(1,segments):
            for y in np.arange(1,segments):
                corners.append([center[1] - size/2 + y*seg_size, center[0] - size/2 + x*seg_size   ])
        return img,np.float32(corners)
        
    

    def run(self):
        center = (self.projector_resolution[0]/2, self.projector_resolution[1]/2)
        no = 6 
        img,actual_corners = self.generate_chessboard(center, no, 300 )        
        print actual_corners
        calabrating = True
        warp = False
        invert = np.array([[-1,0,0], [0,-1,0], [0,0,1] ] , dtype=np.uint8)
        
        min_d = -10.0
        max_d =  10.0
        cv2.namedWindow('sandbox')
        cv2.moveWindow('sandbox',1280,-80)
        
        i = 0
        while True:
            i += 1 
            (depth,got_depth), (rgb,_) = get_depth(), get_video()
            raw_depth = np.asarray(depth, float)
            if i%100==0:
                print "mid pix is : ", depth[w/2.0, h/2.0]
                print "min max : ", depth.max(), depth.min()
                # if not calabrating:
                    # embed()
                # plt.hist(depth.ravel(),256,[0,256]); 
                # plt.show()
            # depth = cv2.warpPerspective(depth,invert, (depth.shape[1], depth.shape[0]))
            # rgb   = cv2.warpPerspective(rgb,invert,(depth.shape[1], depth.shape[0]))
            if not calabrating:
                
                diff = raw_depth - base_depth
                out_range  = (diff < min_d) | (diff > min_d)
                
                
                diff[out_range] = 0.0
                diff = (diff - min_d)*255.0/( max_d - min_d )
                # diff = np.zeros_like(diff)
                
                displydepth = diff.astype(np.uint8)
                if i%100==0:
                    embed()
                
                # depth = cv2.cvtColor(cv2.cvtColor(depth, cv2.COLOR_GRAY2BGR), cv2.COLOR_RGB2LAB )
                # depth[:,:,0] = depth[:,:,0]
                # if not calabrating:
                    # depth = (((depth - base_depth[w/2,h/2])/3000.0)+1)*127.0
                    # depth = (depth - 724)*255/(2047 - 724)
                # depth = ((depth+96)*3).astype(np.uint8) 
                    # depth = depth *255/ (depth.max() - depth.min())
                    
                w = depth.shape[0]
                h = depth.shape[1]
                cv2.imshow('depth', displydepth)
                
            if calabrating:
            
                cv2.imshow('sandbox', img)
                
                found, corners = cv2.findChessboardCorners(rgb,(no-1,no-1))
                if found:
                    corners = corners.reshape(actual_corners.shape)
                    
                    base_depth = raw_depth.copy()
                    
                    transform, status =  cv2.findHomography(corners,actual_corners)
                    calabrating = False
                
            if not calabrating:
                size  = (img.shape[1],img.shape[0])
                transformed= cv2.warpPerspective(depth,transform*invert,size)
                cv2.imshow('sandbox', transformed)
            
            key=cv2.waitKey(30)
            if key==ord('q'):
                break
            

if __name__ == "__main__":
    camera =''
    c = Calabration(camera,20, projector_resolution=(720, 1280))
    c.run()
    
    # with open('calabration.npy') as output :
    #     np.save(output,[c])
    
