# CS543 HW2 Reflection

Xuanyu Chen
xchen9@wpi.edu

### glDrawElements and EBO

previously we use glDrawArrays for drawing. This may requires you to store some vertices twice as some vertices are used in different faces. OpenGL has elements buffer objects (EBO) which allows you to store each vertex only once as long as you also store the correct index.

Using EBO, you need change glDrawArrays to glDrawElements.

### model view projecton
- Shader is our friend, we had better use it to do transform. GLSL program runs in GPU and it is parallel in nature. So trying to forget doing transform sequentially in CPU, trying to do things in GPU (Shader).

- Model matrix is where the transform happens, every translate, rotate, scale should be in here.

- View matrix let you specify the position of your eye, the position you lookat.

- In HW1, we used Ortho2D to produce a orthographic projection, in this assignment we need perspective projection to make z-translation work. 

### Some debug parctice
- Remember, everything within (-1,1) can be seen in the screen. So normally we can first translate it to origin, and scale it to make every coordinate in range (-1,1). Normally you will see pictures shown in screen after these two steps. After you see something, try to apply view matrix and projection matrix.

- How to check if my normal calculation is correct ? Use normal as color, if your normal is correctly calculated, then the entire scene will show very beautiful RGB colors.

### Advanced solution to drawing normal and implementing pulse effects

#### Advanced: Using geometry shader

Using geometry shader may be more inituitive:
- Rendering pipeline, geometry shader runs after vertex shader, before fragment shader, its purpose is to modify vertex information based on the output of vertex shader.

- Refactor the shader loader program to make it support more shaders:
currently InitShaders only support vertex shader and fragment shader, we have to refactor it to make it support more shaders.

[This course lides](https://web.cs.wpi.edu/~emmanuel/courses/cs543/f18/slides/lecture03b.pdf) provides a lot of information on refactoring the shader.

- Geometry Shader:
Hmmmm, I find [this tutorial](https://learnopengl.com/Advanced-OpenGL/Geometry-Shader) to be the best OpenGL tutorial in the web.

Using Geometry shader, calculation of face normal can be done in shader. I will put code in the end so you can see what's really going on.

#### Google Drive Link to some of my implementation using Geometry Shader:
https://drive.google.com/file/d/17w8-Dfmoly5DL9tC76MohBabN7jNZaPq/view?usp=sharing