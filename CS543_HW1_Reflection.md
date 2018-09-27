# CS543 HW1 Reflection

Xuanyu Chen
xchen9@wpi.edu

### How to read data file

The easiest way I can think is using `std::ifstream`. Using `>>` so it can read file one by one. If you need Integer, just declare a variable as int. If you need string, just declare some variable string. If you need float, just declare some variable as float. 

To store data, I use `std::vector`, it is a resizable array in C++. So you dont need some arbitrary large number to declare a global point array.
~~~C++
std::vector<int> indexes;
std::vector<point2> points;

std::ifstream input_file;
input_file.open("model.dat");

int num_polylines;
input_file >> num_polylines;
indexes.push_back(num_polylines);
for(int i = 0; i < num_polylines; i++){
    int num_pixels;
    input_file >> num_pixels;
    indexes.push_back(num_pixels);
    for(int j = 0; j < num_pixels; j++){
        float x; 
        float y;
        input_file >> x;
        input_file >> y;
        points.push_back(point2(x,y));
    }
}
input_file.close()
~~~

### Buffers and Shaders

#### pass data to GPU
The start code uses a global point array `point2 points[3]` tp store vertices for the triangle. Here comes a question, if I use `vector`, how do I send data to buffer.
My Solution is using pointer. If I already have a vector containing a model data. I can use a pointer, and put everything to the array that pointer points to. Then pass that array to GPU.

For example:

~~~C++
// points have model data
float *v = new float[points.size()*2];
int index = 0;
for(int i = 0; i < points.size(); i++){
    v[index++] = points[i].x;
    v[index++] = points[i].y;
}

GLuint VAO;
GLuint VBO;
glGenVertexArrays(1, &VAO);
glGenBuffers(1, &VBO);
glBindVertexArray(VAO);
glBinBuffer(GL_ARRAY_BUFFER, VBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(float)*points.size()*2, v, GL_STATIC_DRAW);
glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
glEnableVertexAttribArray(0);
glBindVertexArray(0);
~~~

#### VBO and VAO
VBO is the actual data that GPU uses. VAO tells GPU how the data in VBO is organized.

For more detail, check the [khronos wiki](https://www.khronos.org/opengl/wiki/Vertex_Specification#Vertex_Array_Object),

#### One Giant Buffer or Different Buffers ?
From my practice, either is fine. But using one giant VBO and VAO is diffcult when you wnat to debug. You need figure out the starting index for each image.

So you can create an array of VAOs and an array of VBOs for different data. This way, in your glDrawArrays, the starting index will always be 0. You just need to bind VAO whenever you pass data and using glDrawArrays.

In first assignment, you can create VAOs like dine_VAO, knight_VAO, usa_VAO, ... but in following assignments we will use 43 3D meshes. Creating things manually is never a good way. So, here we need some abstract thinking and a little bit OOP thinking. 

Why not create a class to represerent model, and in this class does the model loading, and buffer setup ? I will discuss this model class later.

#### uniform and attribute
- They are two ways to send data to GPU
- uniform data remains unchanged for every shader until you change them. attribute are different data for each shader.
- Think this:
    - In our start code, in the file vertex1.glsl. Why `vPosition` is declared as vec4 instead of vec4[], because our data is actually an array ?
    - Answer: Shaders are program runs in Graphics Card, they runs in parallel at the same time. 

#### Change Color using fragment shader
You already know how to pass a matrix to vertex shader using glGetUnfiromLocation, and glUniformMatrix4fv, this is shown in slides. You shouldn't miss that. So why not pass some uniform data to fragment shader which changes when you hit keyboard C. The syntax is same, only difference is that based on the data type you send. choose different glUniform function. For example using glUniform1i if you just send an integer to shader.

### Program Structure and callback function
Think this:
- once the data is loaded from file, it should be there, once it is send to GPU, why bother read it again and send it to GPU again ?
- Shaders are not need to change, what changes is some variable in the shader. So why bother InitShaders many times since shaders wont change ?

So something really only need initialize once, and dont need reload, or re initial every time, call back functions runs in a loop. If you re initshader or re initialize buffer, you may allocate a lot of memory in the graphics card, which is prone to error and crash.

The program structure is simple. First Initial, then register callback function, then go into mainLoop.

Each call back should only do what its supposed to do. Say, if keyboard() is keyboard callback function, it should only receive keyboard, and change some global flag. Trying to load data, or draw when hitting keyboard is not necessary and easy to crash. You can use `glutPostRedisplay()` to force the render loop to go into display call back.

So, I will try to keep my display cal back function as clean as possible. Only glDrawArrays and some necessary update to glViewport, and update projection matrix.

### Aspect ratio and Reshape

`Ortho2D` returns a transform matrix you can use in vertex shader. It actually projects your image to some "drawing area", and this "drawing area" is defined using `glViewport`

in this call back `glutReshapeFunc(resize)`, I only update global width and global height. I will calculate every width and height from the gloabl width and global height instead of some arbitrary width or height.

~~~C++
void resize(GLsize w, GLsize h){
    WINDOW_WIDTH = w;
    WINDOW_HEIGHT = h;
}
~~~

So whats in my Reshape ? Much like what shown in slides, I made some modification. Remember, Reshape calls different `glViewport` based on ratio. So, when you not using reshape, I assume you use `glviewport` for tiles and thumbnails. When you using reshape, just replace every `glViewport` with our Reshape function.

Here is my ReShape function, you can compare it with slides, then you should know what modification I made:
~~~C++
void ReShape(float x, float y, float width, float height, float R) {
	if (R > width / height) // tall viewport
		glViewport(x, y, width, width / R);
	else if (R < width / height) // wide viewport
		glViewport(x, y, height*R, height);
	else
		glViewport(x, y, width, height);
}
~~~

### My model class
Here is the header file for the model class, which defines its interface:
~~~C++
#pragma once
#include<vector>
#include<string>
#include "Angel.h"
#include "drawable.h"

typedef vec2 point2;

class DatModel : public Drawable
{
public:
	DatModel();
	DatModel(const char *filename);
	~DatModel();

	void Render() const override;
	//float xMin, xMax, yMin, yMax;
	//float ratio;

private:
	std::string model_name;
	void ReadFile(const char *filename);
	void BufferSetup();

	std::vector<point2> points;
	std::vector<int> indexes;
	unsigned int VAO;
};
~~~

`Drawable` is a base class. I can extend `Drawable` to create `DatModel` to represent our model from file, and also extend to create Fractal(Fern, GingerBreadMan). I chooses to do this way so I can simplify my code in Main.cpp.

So, this model class has only a public Render() method except the constructor, this is a replacement for glDrawArrays, in my Main.cpp, whenever I call glDrawArrays, I use Render() instead;

Here is my `BufferSetup()` and `Render()`:
~~~C++
void DatModel::BufferSetup() {
	float *v = new float[points.size() * 2];
	int index = 0;
	for (int i = 0; i < points.size(); i++) {
		v[index++] = points[i].x;
		v[index++] = points[i].y;
	}
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*points.size()*2, v, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	delete[] v;

}

void DatModel::Render() const {
	std::cout << "DatModel:: " << model_name << " :: Render()" << std::endl;
	
	glBindVertexArray(VAO);
	//glClear(GL_COLOR_BUFFER_BIT);
	int num_polylines = indexes[0];
	int start = 0;
	for (int i = 0; i < num_polylines; i++) {
		//std::cout << "DatModel:: " << name << " Rendering " << indexes[i + 1] << " Lines" << std::endl;
		glDrawArrays(GL_LINE_STRIP, start, indexes[i+1]);
		start += indexes[i + 1];
	}
	glFlush();
	glBindVertexArray(0);

}

~~~

This way you can laregly simplify your code in the Main.cpp. Here is a snippet in Main.cpp for using this model class.

~~~C++

// Some initial 
std::vector<Drawable*> model_ptr;
std::string models[] = {"birdhead.dat", "dino.dat", "dragon.dat", "house.dat",
						"knight.dat", "rex.dat", "scene.dat", "usa.dat", "vinci.dat"};
std::string dir = "models/";
for (int i = 0; i < 9; i++) {
	std::string file_path = dir + models[i];
	model_ptr.push_back(new DatModel(file_path.c_str()));
}

// DrawTile()
void DrawTile() {

	int num_row = 5;
	int num_col = 5;

	float width = 0.2 * WINDOW_WIDTH;
	float height = 0.2 * WINDOW_HEIGHT;
	
	for (int i = 0; i < num_row; i++) {
		for (int j = 0; j < num_col; j++) {
			int ptr_index = std::rand() % model_ptr.size();
			auto curr_ptr = model_ptr[ptr_index];
			ReShape(i * width, j * height, width, height, curr_ptr->ratio);
			UpdateMatrix(curr_ptr);
			curr_ptr->Render();
		}
	}
}
~~~

`DrawTile()` is then called in display when I need to display the tiles.

### Misc
Using `enum` instead of Integer when necessary.

You can see this google drive for my source code for HW1, note some mouse function is not implemented.
https://drive.google.com/open?id=1c-SCE821tmK1nXNKANIrsDp4dyfbYa9N
