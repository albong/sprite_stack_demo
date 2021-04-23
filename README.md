This is a simple demo of 3d rendering with sprite stacking. I ripped out part of another project for the base of this, and cobbled the logic together from reading tutorials and guessing.

Sprite stacking is where you render a 3D object by drawing 2D images over and over slightly offset in the y-direction to mimic depth - a STACK of 2D SPRITES. Essentially, think about projecting z-slices down into the xy-plane in the direction of the camera.

Of course this has HORRIBLE overdraw, you're drawing images over and over and over just for a single object. And obviously you can't get correct perspective with this. If you could skew a rectangle into a trapezoid then maybe, but you can't do that with SDL alone, and once you're using OpenGL why aren't you just doing real 3D?

BUT. There is one nice advantage of sprite stacking, and that's that you don't need to do any depth sorting. None of the tutorials I saw took advantage of this for some reason - perhaps because its hard to implement with GameMaker? In any case, if you just draw each layer from bottom to top (instead of drawing each object back to front), you never need to know depth. Provided your sprites live in an image atlas, and you are smart with the data structures and caching, you can get okay performance. Plus you can make other optimizations like prerendering the static parts of a layer to reduce draw calls, etc.

I didn't do any billboarded sprites but they're easy enough to add, and while you do have to depth sort them, its still no big deal.
