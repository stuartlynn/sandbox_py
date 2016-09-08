# Sanbox

We love the augmented reality sanbox project from [UC Davis](http://idav.ucdavis.edu/~okreylos/ResDev/SARndbox/)! It's such a wonderful idea 
and makes for a great interative physical interface. We wanted to replicate and build on this idea using OpenFrameworks as it is easily extensiable 
and well used within the community. This is our attempt at doing that 



## Setup 

Simply download the repo here and compile it like you would any OpenFrameworks project. We are trying to keep third party 
addons to Open Frameworks to a minimum but will record any that we use here.

Once the program is running hit C to enter calabration mode. This will project a checkerboard down on to your sanbox for  seccond to 
help the program fine the mapping it needs. 

Once its calabrated you should see a GUI window. try to get the sand as flat as normal and then hit normalize. This will 
take a snapshop of the base level height of the sand. 

Next adjust the upper and lower thresholds so that a whole in the sand all the way down to the base of the sandbox is just within the upper 
range (ie a color show there) and the tallest hill you can build is similarly colored just at the end of the near threshold.

## Work in progress

This is very much a work in progress and any contributions are welcome. Some things we hope to add in the future: 

1. Multiple gradients for the height of the sand 
2. Contour calculation and display 
3. Save contours to geojson 
4. Graphics shaders for different height ranges to visualize water etc
5. Physics engine for simulation on the surface of the sand
