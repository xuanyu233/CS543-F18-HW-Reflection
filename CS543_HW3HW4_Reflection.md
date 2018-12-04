# CS543 HW4 Reflection

Xuanyu Chen
xchen9@wpi.edu

# General OpenGL Program Structure

I Think this question is most important and I have seen this kind of problems through every assignments. People tend to do everything again and again to draw something. For example, create buffer in GPU, calculate data, load data to GPU and draw them. This is common for general programming applications which only happens on CPU. But keep in mind that openGL uses graphics card which runs in parallel in nature. Especially when doing some transform, using graphics card will definitely speed up the overal performance.

Most important thing : 
**Dont Even think of initializing and creating buffers in display callback**

**Display callback runs in mainloop, if you keep allocating space in shader, the applications runs slowly and easy to crash**

In some cases, when you want to do some transform, you only need calculate the transform matrix, passing that matrix to the shader, and let GPU does everything else for you. If you try to calculate every 'new' points from previous points, and load the 'new' points to buffer, and draw again, in most cases you are doing something wrong.

So when it comes to HW3, when we want to draw the 'tree' using L systems, we are actually drawing a line, and transforming the line to different places. Instead of calculating every end points of the tree and draw thousands of lines.

# Smooth Shading Flat Shading

### How to tell difference between phong shading, smooth shading, flat shading ?
Using face normal : flat shading
Using vertex normal : smooth shading

Smooth shading calculated in vertex shader : Gouraud Shading
Smooth shading calculated in fragment shader : Phong Shading

# Phong Shading Implementation
Please check these two tutorials or book chapter if you still get stuck in Phong Lighting:
- [LearnOpenGL](https://learnopengl.com/Lighting/Basic-Lighting)
- [GLSL Cookbook](https://proquest.safaribooksonline.com/9781849514767)

### Why in general shading calculated in fragment shader has better detail in vertex shader ? 
When shading calculation happens in vertex normal, each RGB value for the vertex is "correct", OpenGL will interpolate color value for other pixels. While calculating in fragment shader, the normal is interpolated but the RGB value for each pixel is calculated by you. So normally calculating in fragment shader will require more resources but have better detail compared with vertex shader.


# Calculate Vertex Normals from Face Normals
From definition, the vertex normals are easy to calculate using the following procedure:
- calculate all face normals
- for a given vertex, do:
    - find all faces using this vertex
    - averaging all face normals to get vertex normal

Suppose we have N vertex and M faces, the running time for this naive approach will be O(M) + O(MN). This is okay for small mesh. But budda has too much vertices and faces, this algorithm is extremely slow. So Let's try to improve.

Considering that the main running time is wasted in iterate through each face to find which face has current vertex. So one idea to improve is to only iterate through faces, add current face normal to the three vertex normals. The pseudo code for this algorithm will be :

~~~Python
for vertex in vertices:
    initialize all vertex normals to vec3(0, 0, 0)

for face in faces:
    face_normal = calaulate_face_normal_using_newell_or_cross
    for vertex in face :
        current_vertex_normal += face_normal
~~~

You can do normalzation of vertex normals in vertex shader. So the running time for this improved approach will be O(N) + O(M), which is a great improvement.

# Direction vs Position
In the example shader code given in slides for lighting, we have some code like : ~if(LightPosition.w != 0)~. If you have ever doubted what is w in LightPisition, you will have a better understanding in what is directional light and what is point light.

You may noticed that we use vec4 for almost everything. In glPosition, w is used when calculating final x, y, z value. So it is usually set to 1. However, interms of direction, the w should always be 0. So for a normal, you can use vec3(x, y, z), or vec4(x, y, z, 0).

So the eample above checks if LightPosition is given as a direction or a position.

# PLYModel Class with vertex normals

Finally, after adding the vertex normals, our PLYModel class becomes more mature.
Here is part of the header file for the PLYModel class interface:
~~~C++
class PLYModel : public Drawable {
public:
	<...>
	virtual void Render() const override;
	<...>
private:
	<...>	
};
~~~
