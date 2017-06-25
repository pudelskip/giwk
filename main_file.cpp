/*
Niniejszy program jest wolnym oprogramowaniem; możesz go
rozprowadzać dalej i / lub modyfikować na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundację Wolnego
Oprogramowania - według wersji 2 tej Licencji lub(według twojego
wyboru) którejś z późniejszych wersji.

Niniejszy program rozpowszechniany jest z nadzieją, iż będzie on
użyteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyślnej
gwarancji PRZYDATNOŚCI HANDLOWEJ albo PRZYDATNOŚCI DO OKREŚLONYCH
ZASTOSOWAŃ.W celu uzyskania bliższych informacji sięgnij do
Powszechnej Licencji Publicznej GNU.

Z pewnością wraz z niniejszym programem otrzymałeś też egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
jeśli nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/

#define GLM_FORCE_RADIANS

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include "allmodels.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include <random>

std::random_device r;
std::default_random_engine e1(r());

struct wolne_pola{
int a;
int b;
};

using namespace glm;

float speed_x = 0; // [radiany/s]
float speed_y = 0; // [radiany/s]
float speed1 =0;
float speed2 =-1;

float cam_angle=20;
float cam_speed=0;
bool cam_scr1=false;
bool cam_scr2=true;

bool add=false;
bool can_move=true;

vec4 jedzenie; //położenie "jedzenia"
vec4 przesun[121]; //położenie każdego z segmentów
wolne_pola wolne[121];
int pole[11][11]; //tablica z pozycjami segmentów na planszy 11x11
bool glodny=true;

//Uchwyty na shadery
ShaderProgram *shaderProgram;
ShaderProgram *shaderProgram1;
ShaderProgram *shaderProgramT; //Drugi shader do podłogi

//Uchwyty na VAO i bufory wierzchołków
GLuint vao;
GLuint bufVertices; //Uchwyt na bufor VBO przechowujący tablicę współrzędnych wierzchołków
GLuint bufColors;  //Uchwyt na bufor VBO przechowujący tablicę kolorów
GLuint bufNormals;

GLuint bufVertices1; //Uchwyt na bufor VBO przechowujący tablicę współrzędnych wierzchołków
GLuint bufColors1;  //Uchwyt na bufor VBO przechowujący tablicę kolorów
GLuint bufNormals1;

//podłoga cd.
GLuint vao_plan;
GLuint bufVertices_plan;


GLuint tex0;
GLuint tex1;


 //Uchwyt na bufor VBO przechowujący tablickę wektorów normalnych

//Kostka
float* vertices1=Models::CubeInternal::vertices;
float* colors1=Models::CubeInternal::colors;//
float* normals1=Models::CubeInternal::normals;
int vertexCount1=Models::CubeInternal::vertexCount;

//wierzchołki podłogi (plan)
float ground[]={
    11.0f,-1.0f,11.0f,1.0f,
    -11.0f,-1.0f,11.0f,1.0f,
    11.0f,-1.0f,-11.0f,1.0f,

    -11.0f,-1.0f,-11.0f,1.0f,
    11.0f,-1.0f,-11.0f,1.0f,
    -11.0f,-1.0f,11.0f,1.0f
    };


//Czajnik
float* vertices=Models::TeapotInternal::vertices;
float* colors=Models::TeapotInternal::colors;
float* normals=Models::TeapotInternal::vertexNormals;
int vertexCount=Models::TeapotInternal::vertexCount;




//float przesun[4] = {5.0f,5.0f,0.0f,0.0f};
GLuint readTexture(const char* filename) {
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);

	//Wczytanie do pamięci komputera
	std::vector<unsigned char> image;   //Alokuj wektor do wczytania obrazka
	unsigned width, height;   //Zmienne do których wczytamy wymiary obrazka
	//Wczytaj obrazek
	unsigned error = lodepng::decode(image, width, height, filename);

	if (error!=0) {
        printf("Error while reading texture %s. Error code: %d. \n",filename,error);
	}

	//Import do pamięci karty graficznej
	glGenTextures(1,&tex); //Zainicjuj jeden uchwyt
	glBindTexture(GL_TEXTURE_2D, tex); //Uaktywnij uchwyt
	//Wczytaj obrazek do pamięci KG skojarzonej z uchwytem
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
	GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*) image.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	return tex;
}

class ObjObject{
private:
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<int> vertices_Index;
    std::vector<int> normals_Index;
    std::vector<int> uvs_Index;
    unsigned int vertexCount;
    FILE* fileHandle;
    GLuint vao;
    GLuint bufVertices;
    GLuint bufNormals;
    GLuint bufUvs;
    GLuint texture;
public:
    ObjObject(){}
    ObjObject(const char* fileName, const char* textureName){
        OpenFile(fileName);
        if(fileHandle != NULL){
            Initialize(fileHandle);
            CreateVao();
            fclose(fileHandle);
            texture = readTexture(textureName);
        }

    }
    std::vector<glm::vec3> getVertices(){
        return vertices;
    }
    std::vector<glm::vec3> getNormals(){
        return normals;
    }
    std::vector<glm::vec2> getUvs(){
        return uvs;
    }
    unsigned int getVertexCount(){
        return vertexCount;
    }
    GLuint getVao(){
        return vao;
    }
    GLuint getTexture(){
        return texture;
    }
     ~ObjObject(){
    }
    void OpenFile(const char* fileName){
        fileHandle = fopen(fileName, "r");
        if(fileHandle == NULL){
            std::cout <<"error while opening file " << fileName << std::endl;
        }

    }
    void Initialize(FILE *file){

    char fileLine[256];
        std::vector<glm::vec3> tmpVertices;
        std::vector<glm::vec3> tmpNormals;
        std::vector<glm::vec2> tmpUvs;

        glm::vec3 vertex;
        glm::vec3 normal;
        glm::vec2 uv;
        int status;
        int vertexIndex[3], normalIndex[3], uvIndex[3];
        while( fscanf(file, "%s", fileLine) != EOF){
            if( strcmp(fileLine, "v") == 0){
                status = fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
                if(status != 3){
                    std::cout <<"Error while loading vertex line" << std::endl;
                    break;
                }
                else{
                    tmpVertices.push_back(vertex);
                }
            }
            if( strcmp(fileLine, "vt") == 0){
                status = fscanf(file, "%f %f\n", &uv.x, &uv.y);
                if(status != 2){
                    std::cout <<"Error while loading uv line" << std::endl;
                    break;
                }
                else{
                    uv.y = 1-uv.y;
                    tmpUvs.push_back(uv);
                }
            }
            if( strcmp(fileLine, "vn") == 0){
                status = fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
                if(status != 3){
                    std::cout <<"Error while loading normal line" << std::endl;
                    break;
                }
                else{
                    tmpNormals.push_back(normal);
                }
            }
            if( strcmp(fileLine, "f") == 0){
                status = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0],
                        &vertexIndex[1], &uvIndex[1], &normalIndex[1],
                        &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
                if(status != 9){
                    std::cout <<"Error while loading index line" << std::endl;
                    break;
                }
                else{
                    vertices_Index.push_back(vertexIndex[0]);
                    vertices_Index.push_back(vertexIndex[1]);
                    vertices_Index.push_back(vertexIndex[2]);

                    normals_Index.push_back(normalIndex[0]);
                    normals_Index.push_back(normalIndex[1]);
                    normals_Index.push_back(normalIndex[2]);

                    uvs_Index.push_back(uvIndex[0]);
                    uvs_Index.push_back(uvIndex[1]);
                    uvs_Index.push_back(uvIndex[2]);
                }
            }
        }
        glm::vec3  tmpVertex, tmpNormal;
        glm::vec2 tmpUv;
        for (unsigned int i = 0; i < vertices_Index.size() ; i++){
            tmpVertex = tmpVertices[ vertices_Index[i]-1 ];
            vertices.push_back(tmpVertex);

            tmpNormal = tmpNormals[ normals_Index[i]-1 ];
            normals.push_back(tmpNormal);

            tmpUv = tmpUvs[ uvs_Index[i]-1 ];
            uvs.push_back(tmpUv);
        }
        vertexCount = vertices.size();
    }
    void CreateVao(){
        glGenVertexArrays(1,&vao); //Wygeneruj uchwyt na VAO i zapisz go do zmiennej globalnej
        glBindVertexArray(vao);
        GenerateAndBindBuffer(bufVertices, vertices, 0);
        GenerateAndBindBuffer(bufNormals, normals, 1);
        GenerateAndBindBuffer(bufUvs, uvs, 2);
        glBindVertexArray(0);
    }
    void GenerateAndBindBuffer(GLuint buffer, std::vector<glm::vec3>& data, int attribute){
        glGenBuffers(1,&buffer);
        glBindBuffer(GL_ARRAY_BUFFER,buffer);
        glBufferData(GL_ARRAY_BUFFER, vertexCount* sizeof(glm::vec3) , &data[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(attribute);
        glVertexAttribPointer(
         attribute, 3,GL_FLOAT, GL_FALSE, 0, (void*)0 );
    }
    void GenerateAndBindBuffer(GLuint buffer, std::vector<glm::vec2>& data, int attribute){
        glGenBuffers(1,&buffer);
        glBindBuffer(GL_ARRAY_BUFFER,buffer);
        glBufferData(GL_ARRAY_BUFFER, vertexCount* sizeof(glm::vec2) , &data[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(attribute);
        glVertexAttribPointer(
         attribute, 2,GL_FLOAT, GL_FALSE, 0, (void*)0 );
    }
};

class ObjEntity{
private:
    int id;
    glm::vec3 coords;
    ObjObject* object;
    float scale;
public:
    ObjEntity(){}
    ObjEntity(glm::vec3 coords, int id, float scale, ObjObject* object){
        this->coords = coords;
        this->id = id;
        this->object = object;
        this->scale = scale;
    }
    ~ObjEntity(){}
    int getId(){
        return id;
    }
    glm::vec3 getCoords(){
        return coords;
    }
    ObjObject* getObject(){
        return object;
    }
    float getScale(){
        return scale;
    }
};

void loadObjectFromFile(){
}



//Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

//Procedura obsługi klawiatury
void key_callback(GLFWwindow* window, int key,
	int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_LEFT && speed1==0 &&can_move){ speed1 = -1.0;speed2 = 0.0; can_move=false;}
		if (key == GLFW_KEY_RIGHT && speed1==0 &&can_move){ speed1 = 1.0;speed2 = 0.0;can_move=false;}
		if (key == GLFW_KEY_UP && speed2==0 &&can_move){ speed1 = 0.0;speed2 = -1.0;can_move=false;}
		if (key == GLFW_KEY_DOWN && speed2==0 &&can_move){ speed1 = 0.0;speed2 = 1.0;can_move=false;}

		if (key == GLFW_KEY_P && cam_angle<20){cam_speed=2;}
		if (key == GLFW_KEY_L && cam_angle>15){cam_speed=-2;}
	}
    if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_P ){cam_speed=0;}
		if (key == GLFW_KEY_L ){cam_speed=0;}
    }

}

//Tworzy bufor VBO z tablicy
GLuint makeBuffer(void *data, int vertexCount, int vertexSize) {
	GLuint handle;

	glGenBuffers(1,&handle);//Wygeneruj uchwyt na Vertex Buffer Object (VBO), który będzie zawierał tablicę danych
	glBindBuffer(GL_ARRAY_BUFFER,handle);  //Uaktywnij wygenerowany uchwyt VBO
	glBufferData(GL_ARRAY_BUFFER, vertexCount*vertexSize, data, GL_STATIC_DRAW);//Wgraj tablicę do VBO

	return handle;
}

//Przypisuje bufor VBO do atrybutu
void assignVBOtoAttribute(ShaderProgram *shaderProgram,const char* attributeName, GLuint bufVBO, int vertexSize) {
	GLuint location=shaderProgram->getAttribLocation(attributeName); //Pobierz numery slotów dla atrybutu
	glBindBuffer(GL_ARRAY_BUFFER,bufVBO);  //Uaktywnij uchwyt VBO
	glEnableVertexAttribArray(location); //Włącz używanie atrybutu o numerze slotu zapisanym w zmiennej location
	glVertexAttribPointer(location,vertexSize,GL_FLOAT, GL_FALSE, 0, NULL); //Dane do slotu location mają być brane z aktywnego VBO
}


//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {

	//************Tutaj umieszczaj kod, który należy wykonać raz, na początku programu************
	glClearColor(0, 0, 0, 1); //Czyść ekran na czarno
	glEnable(GL_DEPTH_TEST); //Włącz używanie Z-Bufora
	glfwSetKeyCallback(window, key_callback); //Zarejestruj procedurę obsługi klawiatury


	shaderProgram=new ShaderProgram("vshader.txt",NULL,"fshader.txt"); //Wczytaj program cieniujący
    shaderProgram1=new ShaderProgram("vshader1.txt",NULL,"fshader1.txt");
    shaderProgramT=new ShaderProgram("vshader2.txt",NULL,"fshader2.txt");

	//*****Przygotowanie do rysowania pojedynczego obiektu*******
	//Zbuduj VBO z danymi obiektu do narysowania

	//bufory glowy
	bufVertices1=makeBuffer(vertices1, vertexCount1, sizeof(float)*4); //VBO ze współrzędnymi wierzchołków
	bufColors1=makeBuffer(colors1, vertexCount1, sizeof(float)*4);//VBO z kolorami wierzchołków
	bufNormals1=makeBuffer(normals1, vertexCount1, sizeof(float)*4);//VBO z wektorami normalnymi wierzchołków

    bufVertices=makeBuffer(vertices, vertexCount, sizeof(float)*4); //VBO ze współrzędnymi wierzchołków
	bufColors=makeBuffer(colors, vertexCount, sizeof(float)*4);//VBO z kolorami wierzchołków
	bufNormals=makeBuffer(normals, vertexCount, sizeof(float)*4);//VBO z wektorami normalnymi wierzchołków

    //bufor podlogi
    bufVertices_plan= makeBuffer(ground, 6, sizeof(float)*4);


	//Zbuduj VAO wiążący atrybuty z konkretnymi VBO
	glGenVertexArrays(1,&vao); //Wygeneruj uchwyt na VAO i zapisz go do zmiennej globalnej
    glGenVertexArrays(1,&vao_plan);

/*
	glBindVertexArray(vao); //Uaktywnij nowo utworzony VAO

	assignVBOtoAttribute(shaderProgram,"vertex",bufVertices,4); //"vertex" odnosi się do deklaracji "in vec4 vertex;" w vertex shaderze
	assignVBOtoAttribute(shaderProgram,"color",bufColors,4); //"color" odnosi się do deklaracji "in vec4 color;" w vertex shaderze
	assignVBOtoAttribute(shaderProgram,"normal",bufNormals,4);
	//"normal" odnosi się do deklaracji "in vec4 normal;" w vertex shaderze

	glBindVertexArray(0);
*/
	//VAO podlogi
	glBindVertexArray(vao_plan); //Uaktywnij nowo utworzony VAO

	assignVBOtoAttribute(shaderProgram1,"vertices",bufVertices_plan,4);

	glBindVertexArray(0);
	//Dezaktywuj VAO

	tex0=readTexture("metal.png");
	tex1=readTexture("metal_spec.png");

}

//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram() {
	delete shaderProgram;
	delete shaderProgram1; //Usunięcie programu cieniującego

    glDeleteVertexArrays(1,&vao_plan); //Usunięcie vao
	glDeleteBuffers(1,&bufVertices_plan);

	glDeleteVertexArrays(1,&vao); //Usunięcie vao
	glDeleteBuffers(1,&bufVertices); //Usunięcie VBO z wierzchołkami
	glDeleteBuffers(1,&bufColors); //Usunięcie VBO z kolorami
	glDeleteBuffers(1,&bufNormals); //Usunięcie VBO z wektorami normalnymi


}

void drawObject(GLuint vao, ShaderProgram *shaderProgram, mat4 mP, mat4 mV, mat4 mM, vec4 Przes) {
	//Włączenie programu cieniującego, który ma zostać użyty do rysowania
	//W tym programie wystarczyłoby wywołać to raz, w setupShaders, ale chodzi o pokazanie,
	//że mozna zmieniać program cieniujący podczas rysowania jednej sceny
	shaderProgram->use();

	//Przekaż do shadera macierze P,V i M.
	//W linijkach poniżej, polecenie:
	//  shaderProgram->getUniformLocation("P")
	//pobiera numer slotu odpowiadającego zmiennej jednorodnej o podanej nazwie
	//UWAGA! "P" w powyższym poleceniu odpowiada deklaracji "uniform mat4 P;" w vertex shaderze,
	//a mP w glm::value_ptr(mP) odpowiada argumentowi  "mat4 mP;" TYM pliku.
	//Cała poniższa linijka przekazuje do zmiennej jednorodnej P w vertex shaderze dane z argumentu mP niniejszej funkcji
	//Pozostałe polecenia działają podobnie.
	glUniformMatrix4fv(shaderProgram->getUniformLocation("P"),1, false, glm::value_ptr(mP));
	glUniformMatrix4fv(shaderProgram->getUniformLocation("V"),1, false, glm::value_ptr(mV));
	glUniformMatrix4fv(shaderProgram->getUniformLocation("M"),1, false, glm::value_ptr(mM));
	glUniform4fv(shaderProgram->getUniformLocation("Przes"),1, glm::value_ptr(Przes));//przesuniecie aktualnie rysowanego segmentu
	//glUniform4fv(shaderProgram->getUniformLocation("prz"),1, przesun);

	//Uaktywnienie VAO i tym samym uaktywnienie predefiniowanych w tym VAO powiązań slotów atrybutów z tablicami z danymi
	glBindVertexArray(vao);

	//Narysowanie obiektu
	glDrawArrays(GL_TRIANGLES,0,vertexCount);

	//Posprzątanie po sobie (niekonieczne w sumie jeżeli korzystamy z VAO dla każdego rysowanego obiektu)
	glBindVertexArray(0);
}

void drawObject2(GLuint vao, ShaderProgram *shaderProgram, mat4 mP, mat4 mV, mat4 mM) {

	shaderProgram->use();


	glUniformMatrix4fv(shaderProgram->getUniformLocation("P"),1, false, glm::value_ptr(mP));
	glUniformMatrix4fv(shaderProgram->getUniformLocation("V"),1, false, glm::value_ptr(mV));
	glUniformMatrix4fv(shaderProgram->getUniformLocation("M"),1, false, glm::value_ptr(mM));

	glBindVertexArray(vao);

	//Narysowanie obiektu
	glDrawArrays(GL_TRIANGLES,0,6);

	//Posprzątanie po sobie (niekonieczne w sumie jeżeli korzystamy z VAO dla każdego rysowanego obiektu)
	glBindVertexArray(0);
}

void drawObjectT(ObjObject* object, ShaderProgram *shaderProgram, mat4 mP, mat4 mV, mat4 mM,vec4 Przes) {

	shaderProgram->use();

	//Pozostałe polecenia działają podobnie.
	glUniformMatrix4fv(shaderProgram->getUniformLocation("P"),1, false, glm::value_ptr(mP));
	glUniformMatrix4fv(shaderProgram->getUniformLocation("V"),1, false, glm::value_ptr(mV));
	glUniformMatrix4fv(shaderProgram->getUniformLocation("M"),1, false, glm::value_ptr(mM));
    glUniform4fv(shaderProgram->getUniformLocation("Przes"),1, glm::value_ptr(Przes));
	glUniform1i(shaderProgram->getUniformLocation("textureMap0"),0);
	glUniform1i(shaderProgram->getUniformLocation("textureMap1"),1);
	//Przypisanie tekstur do jednostek teksturujących
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,object->getTexture());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,tex1);

	//Uaktywnienie VAO i tym samym uaktywnienie predefiniowanych w tym VAO powiązań slotów atrybutów z tablicami z danymi
    glBindVertexArray(object->getVao());

	//Narysowanie obiektu
	glDrawArrays(GL_TRIANGLES,0,object->getVertexCount());

	//Posprzątanie po sobie (niekonieczne w sumie jeżeli korzystamy z VAO dla każdego rysowanego obiektu)
	glBindVertexArray(0);
}
//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window,float mov1,float mov2,bool moved, int n,ObjEntity *entities) {
	//************Tutaj umieszczaj kod rysujący obraz******************l

    glm::vec3 Obserwator = vec3(0.0f,cam_angle,-3*cam_angle+60);
   // glm::vec3 Obserwator = vec3(0.0f,-0.9f,18.0f);
    glm::vec3 Punkt = vec3(0.0f,0.0f,0.0f);
    glm::vec3 Nos = vec3(0.0f, 1.0f, -1.0f);


    float near=1.0f;
    float far=50.0f;
    float right=1.0f;
    float left=-1.0f;
    float top=1.0f;
    float bottom=-1.0f;

    glm::vec3 Z_obs = normalize(Obserwator-Punkt);
    glm::vec3 X_obs = normalize(cross(Nos,Z_obs));
    glm::vec3 Y_obs = cross(Z_obs,X_obs);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); //Wykonaj czyszczenie bufora kolorów

	glm::mat4 V_1 = glm::mat4(vec4(X_obs,0),vec4(Y_obs,0),vec4(Z_obs,0),vec4(Obserwator,1) );
	//V_1=  transpose(V_1);
	glm::mat4 V = inverse(V_1);
	glm::mat4 P = glm::perspective(50 * PI / 180, 1.0f, 1.0f, 50.0f); //Wylicz macierz rzutowania

	glm::mat4 P_1 = mat4(0.0f);
    P_1[0].x= 2*near/(right-left);
    P_1[1].y= 2*near/(top-bottom);
    P_1[2].x=(right+left)/(right-left);
    P_1[2].y=(top+bottom)/(top-bottom);
    P_1[2].z=-(far+near)/(far-near);
    P_1[2].w=-1;
    P_1[3].z=-(2*near*far)/(far-near);
    P = P_1;
    //n,f,r,b,l,


	//Wylicz macierz modelu rysowanego obiektu


    //Atrybuty głowy
    glBindVertexArray(vao); //Uaktywnij nowo utworzony VAO
	assignVBOtoAttribute(shaderProgram,"vertex",bufVertices1,4); //"vertex" odnosi się do deklaracji "in vec4 vertex;" w vertex shaderze
	assignVBOtoAttribute(shaderProgram,"color",bufColors1,4); //"color" odnosi się do deklaracji "in vec4 color;" w vertex shaderze
	assignVBOtoAttribute(shaderProgram,"normal",bufNormals1,4);
	glBindVertexArray(0);

    glm::mat4 M = glm::mat4(1.0f);
    drawObjectT(entities[0].getObject(),shaderProgramT,P,V,M,przesun[0]);




    //atrybuty ciała

	glBindVertexArray(vao); //Uaktywnij nowo utworzony VAO
	assignVBOtoAttribute(shaderProgram,"vertex",bufVertices1,4); //"vertex" odnosi się do deklaracji "in vec4 vertex;" w vertex shaderze
	assignVBOtoAttribute(shaderProgram,"color",bufColors1,4); //"color" odnosi się do deklaracji "in vec4 color;" w vertex shaderze
	assignVBOtoAttribute(shaderProgram,"normal",bufNormals1,4);
	//"normal" odnosi się do deklaracji "in vec4 normal;" w vertex shaderze
	glBindVertexArray(0);


	for(int i=1;i<n;i++){
        glm::mat4 M = glm::mat4(1.0f);
        //Liczenie przesuniecia jest teraz w vshaderze
        /*glm:mat4 Move = glm::mat4(1.0f);
        Move = Move;
        Move[3].x=przesun[i].x;
        Move[3].z=przesun[i].z;
        M=Move;*/
        //M = glm::translate(M,przesun[i]);

        drawObject(vao,shaderProgram,P,V,M,przesun[i]);

	}

    if(glodny){
       glm::mat4 M = glm::mat4(1.0f);
       // M = glm::translate(M,jedzenie);

        drawObject(vao,shaderProgram,P,V,M,jedzenie);

    }
	//M = glm::rotate(M, angle_x, glm::vec3(1, 0, 0));
	//M = glm::rotate(M, angle_y, glm::vec3(0, 1, 0));

	//Narysuj obiekt
	//drawObject(vao,shaderProgram,P,V,M);

	//Przerzuć tylny bufor na przedni

	//rysoawnie podłogi
	M = glm::mat4(1.0f);
     drawObject2(vao_plan,shaderProgram1,P,V,M);


	glfwSwapBuffers(window);

}



int main(void)
{

    int n=3;// 3 segmenty na poczatku
    int wylosowana; //do losowanie gdzie bedzie jedzenie
	GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno

	glfwSetErrorCallback(error_callback);

	if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(800, 800, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

	if (!window) //Jeżeli okna nie udało się utworzyć, to zamknij program
	{
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekę GLEW
		fprintf(stderr, "Nie można zainicjować GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Operacje inicjujące

	ObjObject objectArray[3] = {{"Snake_headv5.obj", "snake.png"}, {"board.obj", "board.png"}, {"fence.obj", "fence.png"}};
    ObjEntity entityArray[3] = {{vec3(0,1,0),1, 0.7, &objectArray[0]}, {vec3(0,0,0),1 , 1, &objectArray[1]}, {vec3(0,0,0),1 ,1 , &objectArray[2]}};

	//poczatkowe pozyce segmentow
    przesun[0]=vec4(0.0f,0.0f,0.0f,1.0f);
    przesun[1]=vec4(0.0f,0.0f,2.0f,0.0f);
    przesun[2]=vec4(0.0f,0.0f,4.0f,0.0f);

    //poczatkowa pozycja jedzenia
    jedzenie = vec4(0.0f,0.0f,-6.0f,0.0f);

    for(int i=0;i<11;i++)
        for(int j=0;j<11;j++)
            pole[i][j]=0;


    //wpisanie poczakowych pozycje to tablicy z planszą 11x11
    for(int i=0;i<n;i++){
                  pole[int(przesun[i].x/2)+5][int(przesun[i].z/2)+5]=1;
                  //printf("[%d,%d]",int(przesun[i].x/2)+5,int(przesun[i].z/2)+5);
                }

	float mov1=0;
	float mov2=0;

	glfwSetTime(0);
	 //Wyzeruj licznik czasu
    float czas=0;
	//Główna pętla
	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
	{

        bool moved=false;

        //do ruchu kamerą
        if(cam_angle>20) cam_angle=20;
        if(cam_angle<15) cam_angle=15;
        czas += glfwGetTime();
        cam_angle+=cam_speed*glfwGetTime();
        glfwSetTime(0);

		if(czas>1.4f)
        {
            czas=0;
            //glfwSetTime(0); //Wyzeruj licznik czasu
            mov1+=speed1*2;
            mov2+=speed2*2;


            for(int i=n;i>0;i--){
                przesun[i]=przesun[i-1];
                przesun[i].w=0;
            }


            if(speed1==1.0f){przesun[0].y=1; przesun[0].w=4;}
            if(speed1==-1.0f){przesun[0].y=1; przesun[0].w=2;}
            if(speed2==1.0f){przesun[0].y=0; przesun[0].w=3;}
            if(speed2==-1.0f){przesun[0].y=0; przesun[0].w=1;}

            // !!!! Y TO NIE PRZESUNIECIE TYLKO MOWI ZE BEDZIE OBROCONE
            //Zmienna "w" okresla obrot glowy, 0 oznacza ze to segment ciała i obrot jest brany z "y" w vshaderze


            przesun[0].z=mov2;
            przesun[0].x=mov1;

//TODO maybe//dodanie petli obracania wszystkich segmentow
            int pom1 = int(przesun[n-2].x-przesun[n-1].x);
            int pom2 = int(przesun[n-2].z-przesun[n-1].z);
           // printf("%f | %f\n",przesun[n-3].x,przesun[n-2].x);
           printf("%d | %d\n",pom1,pom2);
            if(pom1>0) przesun[n-1].w=3;
            if(pom1<0) przesun[n-1].w=1;
            if(pom2>0) przesun[n-1].w=2;
            if(pom2<0) przesun[n-1].w=4;


            for(int i=1;i<n-1;i++){
                if(przesun[i].y!=przesun[i-1].y)
                przesun[i].w=1.5;
                else
                przesun[i].w=0;
            }




            moved=true;

            for(int i=1;i<n;i++){
                if(przesun[0].x==przesun[i].x && przesun[0].z==przesun[i].z){

                    freeOpenGLProgram();
                    glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
                    glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
                    exit(EXIT_SUCCESS);
                }
            }




            if(przesun[0].x==jedzenie.x && przesun[0].z==jedzenie.z){

                int ile=0;
                for(int i=0;i<11;i++){
                    for(int j=0;j<11;j++){
                            if(pole[i][j]==0){
                            wolne[ile].a=i;
                            wolne[ile].b=j;
                            ile++;
                            }
                    }
                }
                std::uniform_int_distribution<int> uniform_dist(0, ile);
                wylosowana = uniform_dist(e1);
                jedzenie.x=(wolne[wylosowana].a-5)*2;
                jedzenie.z=(wolne[wylosowana].b-5)*2;
               // printf("%d",wylosowana);
                n+=1;
                 int pom1 = int(przesun[n-2].x-przesun[n-1].x);
            int pom2 = int(przesun[n-2].z-przesun[n-1].z);
           // printf("%f | %f\n",przesun[n-3].x,przesun[n-2].x);

            if(pom1>0) przesun[n-1].w=3;
            if(pom1<0) przesun[n-1].w=1;

            if(pom2>0) przesun[n-1].w=2;
            if(pom2<0) przesun[n-1].w=4;
            }
        //włącznie wersji kosolowej dla Intel graphics
        /*
            pole[int(przesun[0].x/2)+5][int(przesun[0].z/2)+5]=1;
            pole[int(przesun[n].x/2)+5][int(przesun[n].z/2)+5]=0;
                printf("\n");
                for(int i=0;i<11;i++){
                    for(int j=0;j<11;j++){
                            printf("%d",pole[j][i]);}
                    printf("\n");
                }
                printf("\n");
        */
         can_move=true;
        }

		drawScene(window,mov1,mov2,moved,n, entityArray); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.

	}

	freeOpenGLProgram();

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
