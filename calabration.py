import numpy as np
import cv2 
from IPython import embed
from freenect import sync_get_depth as get_depth, sync_get_video as get_video

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
        
        
    def     
    def run(self):
        center = (self.projector_resolution[0]/2, self.projector_resolution[1]/2)
        no = 6 
        img,actual_corners = self.generate_chessboard(center, no, 300 )        
        
        calabrating = True
        warp = False
        invert = np.array([[-1,0,0], [0,-1,0], [0,0,1] ] , dtype=np.uint8)
        cv2.namedWindow('sandbox')
        cv2.moveWindow('sandbox',1280,-80)
        while True:
            (depth,got_depth), (rgb,_) = get_depth(), get_video()
            depth = depth.astype(np.uint8)
            depth = cv2.cvtColor(v2.cvtColor(depth, cv.CV_GRAY2RGB), cv.CV_RGB2Lab)
        
            cv2.imshow('depth', depthCol)
            if calabrating:
                cv2.imshow('sandbox', img)
                
                found, corners = cv2.findChessboardCorners(rgb,(no-1,no-1))
                if found:
                    corners = corners.reshape(actual_corners.shape)
                    transform, status =  cv2.findHomography(corners,actual_corners)
                    transform = transform 
                    calabrating = False
                
            if not calabrating:
                size  = (img.shape[1],img.shape[0])
                transformed= cv2.warpPerspective(depthCol,transform,size)
                cv2.imshow('sandbox', transformed)
            
            key=cv2.waitKey(30)
            if key==ord('q'):
                break
            

if __name__ == "__main__":
    camera =''
    c = Calabration(camera,20, projector_resolution=(720, 1280))
    transformation_matrix = c.run()
    
    # with open('calabration.npy') as output :
    #     np.save(output,[c])
    
