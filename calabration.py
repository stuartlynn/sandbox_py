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
        
    def generate_points(self):
        circle_radius = 20
        
        img = np.zeros((self.projector_resolution[0],self.projector_resolution[1],3), np.uint8)
        
        x = np.random.randint(circle_radius,self.projector_resolution[0]-circle_radius ,self.no_points)
        y = np.random.randint(circle_radius,self.projector_resolution[1]-circle_radius,self.no_points)
        color = '#FF0000' 
        for center in zip(x,y):
            cv2.circle(img,center,20, color=(255,0,0), thickness=-1)
        return img
        
    # def detect_points(self):
    #     self.projector_points = 
    #     self.points = cv2.
    #     
    # def generate_transform(self):
        
        
    def run(self):
        center = (self.projector_resolution[0]/2, self.projector_resolution[1]/2)
        no = 6 
        img,actual_corners = self.generate_chessboard(center, no, 300 )

    

        
    
        # cv2.imshow('transformed', transformed)
        
        calabrating = True
        warp = False
        while True:
            (depth,got_depth), (rgb,_) = get_depth(), get_video()
            maxd = np.max(depth)
            mind = np.min(depth)
            print "do we have depth ? ", got_depth, np.shape(depth), np.min(depth), np.max(depth)
            
            depthWidth = np.shape(depth)[0]
            depthHeight = np.shape(depth)[1]
            depth = depth.astype(np.uint8)
            # depth = cv2.flip(depth,1)
            
            
            # depth = np.fliplr(depth)
            # rgb= np.fliplr(rgb)
            depthCol = np.zeros((depthWidth, depthHeight, 3), dtype=np.uint8)
            for x in np.arange(depthWidth):
                for y in np.arange(depthHeight):
                    depthCol[x,y,0] = depth[x,y]
                    depthCol[x,y,1] = 2000-depth[x,y]
                    depthCol[x,y,1] = depth[x,y]
                    
            depthCol   = cv2.flip(depthCol,0)
            depthCol   = cv2.flip(depthCol,1)
            
            rgb   = cv2.flip(rgb,0)
            rgb   = cv2.flip(rgb,1)        
            # for x in np.arange(depthWidth):
            #     for y in np.arange(depthHeight):
            #         depthCol[x,y,0] = depth[x,y]*256.0/2047.0
            #         depthCol[x,y,1] = depth[x,y]*256.0/2047.0
            #         depthCol[x,y,2] = depth[x,y]*256.0/2047.0
                    
            # depth2 = [ [a,a,a] for a in depth ]
            cv2.imshow('depth', depthCol)
            if calabrating:
                print 'calabrating'
                cv2.imshow('checker', img)
                cv2.moveWindow('checker',1280,-80)
                found, corners = cv2.findChessboardCorners(rgb,(no-1,no-1))
                if found:
                    corners = corners.reshape(actual_corners.shape)
                    transform, status =  cv2.findHomography(corners,actual_corners)
                    calabrating = False
                
            if not calabrating:
                cv2.drawChessboardCorners(depthCol,(no-1,no-1),corners,found)
                cv2.imshow('detected', depthCol)
                size  = (img.shape[1],img.shape[0])
                transformed= cv2.warpPerspective(depthCol,transform,size)
                # transformed_depth= cv2.warpPerspective(depth,transform,size)
                cv2.imshow('checker', transformed)
                # cv2.imshow('depth_warped', transformed_depth)
                                
            # cv2.imshow('depth', depth)
            # cv2.imshow('rgb', rgb)
            key=cv2.waitKey(30)
            if key==ord('q'):
                break
            if key==ord('w'):
                warp=True   
            
        # self.detect_points()
        # return generate_transform()

if __name__ == "__main__":
    camera =''
    c = Calabration(camera,20, projector_resolution=(720, 1280))
    transformation_matrix = c.run()
    
    # with open('calabration.npy') as output :
    #     np.save(output,[c])
    
