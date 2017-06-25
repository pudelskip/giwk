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
#include <iostream>
std::vector<glm::vec3> vertices;
std::vector<glm::vec3> normals;
std::vector<glm::vec2> uvs;
GLuint vao;
GLuint bufVertices;
unsigned int vertexCount;

using namespace glm;

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



float speed_x = 0; // [radiany/s]
float speed_y = 0; // [radiany/s]

//Uchwyty na shadery
ShaderProgram *shaderProgram; //Wskaźnik na obiekt reprezentujący program cieniujący.

//Uchwyty na VAO i bufory wierzchołków
//GLuint vao;
//GLuint bufVertices; //Uchwyt na bufor VBO przechowujący tablicę współrzędnych wierzchołków
//GLuint bufColors;  //Uchwyt na bufor VBO przechowujący tablicę kolorów
//GLuint bufNormals; //Uchwyt na bufor VBO przechowujący tablicę wektorów normalnych
//GLuint bufTexCoords; //Uchwyt na bufor VBO przechowujący tablicę współrzędnych teksturowania

//Kostka
/*float* vertices=Models::CubeInternal::vertices;
float* colors=Models::CubeInternal::colors;
float* normals=Models::CubeInternal::normals;
float* texCoords=Models::CubeInternal::texCoords;
//float* normals=Models::CubeInternal::vertexNormals;
int vertexCount=Models::CubeInternal::vertexCount;
*/

//Czajnik
/*float* vertices=Models::TeapotInternal::vertices;
float* colors=Models::TeapotInternal::colors;
//float* normals=Models::TeapotInternal::normals;
float* normals=Models::TeapotInternal::vertexNormals;
float* texCoords=Models::TeapotInternal::texCoords;
int vertexCount=Models::TeapotInternal::vertexCount;
*/
//Snake


//Uchwyty na tekstury
GLuint tex0;
GLuint tex1;


//Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

//Procedura obsługi klawiatury
void key_callback(GLFWwindow* window, int key,
	int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_LEFT) speed_y = -3.14;
		if (key == GLFW_KEY_RIGHT) speed_y = 3.14;
		if (key == GLFW_KEY_UP) speed_x = -3.14;
		if (key == GLFW_KEY_DOWN) speed_x = 3.14;
	}


	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_LEFT) speed_y = 0;
		if (key == GLFW_KEY_RIGHT) speed_y = 0;
		if (key == GLFW_KEY_UP) speed_x = 0;
		if (key == GLFW_KEY_DOWN) speed_x = 0;
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

	shaderProgram=new ShaderProgram("vshader2.txt",NULL,"fshader2.txt"); //Wczytaj program cieniujący

	//*****Przygotowanie do rysowania pojedynczego obiektu*******
	//Zbuduj VBO z danymi obiektu do narysowania
	//bufVertices=makeBuffer(vertices, vertexCount, sizeof(float)*4); //VBO ze współrzędnymi wierzchołków
	//bufColors=makeBuffer(colors, vertexCount, sizeof(float)*4);//VBO z kolorami wierzchołków
	//bufNormals=makeBuffer(normals, vertexCount, sizeof(float)*4);//VBO z wektorami normalnymi wierzchołków
	//bufTexCoords=makeBuffer(texCoordinates, vertexCount, sizeof(float)*2);//VBO ze wspolrzednymi teksturowania


//vertices.size() * sizeof(glm::vec3)
//	glGenBuffers(1,&bufVertices);//Wygeneruj uchwyt na Vertex Buffer Object (VBO), który będzie zawierał tablicę danych
//	glBindBuffer(GL_ARRAY_BUFFER,bufVertices);  //Uaktywnij wygenerowany uchwyt VBO
//	glBufferData(GL_ARRAY_BUFFER, vertexCount* sizeof(glm::vec3) , &vertices[0], GL_STATIC_DRAW);//Wgraj tablicę do VBO
/*
	glGenBuffers(1,&bufNormals);//Wygeneruj uchwyt na Vertex Buffer Object (VBO), który będzie zawierał tablicę danych
	glBindBuffer(GL_ARRAY_BUFFER,bufNormals);  //Uaktywnij wygenerowany uchwyt VBO
	glBufferData(GL_ARRAY_BUFFER, normals.size() , &normals[0], GL_STATIC_DRAW);//Wgraj tablicę do VBO

	glGenBuffers(1,&bufUvs);//Wygeneruj uchwyt na Vertex Buffer Object (VBO), który będzie zawierał tablicę danych
	glBindBuffer(GL_ARRAY_BUFFER,bufUvs);  //Uaktywnij wygenerowany uchwyt VBO
	glBufferData(GL_ARRAY_BUFFER, uvs.size() , &uvs[0], GL_STATIC_DRAW);//Wgraj tablicę do VBO
*/
	//Zbuduj VAO wiążący atrybuty z konkretnymi VBO
	glGenVertexArrays(1,&vao); //Wygeneruj uchwyt na VAO i zapisz go do zmiennej globalnej

	glBindVertexArray(vao); //Uaktywnij nowo utworzony VAO


	//assignVBOtoAttribute(shaderProgram,"vertex",bufVertices,4); //"vertex" odnosi się do deklaracji "in vec4 vertex;" w vertex shaderze
	//assignVBOtoAttribute(shaderProgram,"color",bufColors,4); //"color" odnosi się do deklaracji "in vec4 color;" w vertex shaderze
	//assignVBOtoAttribute(shaderProgram,"normal",bufNormals,4); //"normal" odnosi się do deklaracji "in vec4 normal;" w vertex shaderze
	//assignVBOtoAttribute(shaderProgram,"texCoords",bufUvs,2); //"texCoords" odnosi się do deklaracji "in vec2 texCoords;" w vertex shaderze

	glBindVertexArray(0); //Dezaktywuj VAO
	//******Koniec przygotowania obiektu************

	//Wczytanie tekstur
	tex0=readTexture("metal.png");
	tex1=readTexture("metal_spec.png");
}

//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram() {
	delete shaderProgram; //Usunięcie programu cieniującego

	glDeleteVertexArrays(1,&vao); //Usunięcie vao
	//glDeleteBuffers(1,&bufVertices); //Usunięcie VBO z wierzchołkami
	//glDeleteBuffers(1,&bufColors); //Usunięcie VBO z kolorami
	//glDeleteBuffers(1,&bufNormals); //Usunięcie VBO z wektorami normalnymi
	//glDeleteBuffers(1,&bufTexCoords); //Usunięcie VBO ze współrzędnymi teksturowania

	//Usuń tekstury
	glDeleteTextures(1,&tex0);
	glDeleteTextures(1,&tex1);

}

void drawObject(ObjObject* object, ShaderProgram *shaderProgram, mat4 mP, mat4 mV, mat4 mM) {
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
	//Przekaż współrzędne światła do zmiennej jednorodnej "lp". Obydwie poniższe linijki działają tak samo
	//glUniform4f(shaderProgram->getUniformLocation("lp"),0,0,-6,1); //Kolejne argumenty to kolejne wartości wektora
	//glUniform4fv(shaderProgram->getUniformLocation("lp"),1,value_ptr(vec4(0,0,-6,1))); //drugi argument - przesylamy tylko jeden wektor, trzeci argument - wektor do przesłania

	//Przypisanie jednostek teksturujących do zmiennych
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
void drawScene(GLFWwindow* window, float angle_x, float angle_y, ObjEntity *entities) {
	//************Tutaj umieszczaj kod rysujący obraz******************l

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); //Wykonaj czyszczenie bufora kolorów
    glClearColor(0.9,0.9,0.9,1);
	glm::mat4 P = glm::perspective(50 * PI / 180, 1.0f, 1.0f, 50.0f); //Wylicz macierz rzutowania

	glm::mat4 V = glm::lookAt( //Wylicz macierz widoku
		glm::vec3(0.0f, 0.1f, 10.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

    for(int i =0 ; i<3 ; i++){
	//Wylicz macierz modelu rysowanego obiektu
        glm::mat4 I,R,T,S,M;
        I = glm::mat4(1.0f);
        R = glm::rotate(I, angle_x, glm::vec3(1, 0, 0));
        R = glm::rotate(R, angle_y, glm::vec3(0, 1, 0));
        T = glm::translate(I, entities[i].getCoords());
        S = glm::scale(I, vec3(entities[i].getScale(), entities[i].getScale(), entities[i].getScale()));
        M = R*T*S;
        //Narysuj obiekt
        drawObject(entities[i].getObject(),shaderProgram,P,V,M);
    }

	glfwSwapBuffers(window);

}



int main(void)
{
	GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno
    double deltaTime, lastTime, frameTime;
    int framesCount = 0;
	glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów

	if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(1000, 800, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

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

    ObjObject objectArray[3] = {{"Snake_headv5.obj", "snake2.png"}, {"board.obj", "board.png"}, {"fence.obj", "fence.png"}};
    ObjEntity entityArray[3] = {{vec3(0,1,0),1, 0.7, &objectArray[0]}, {vec3(0,0,0),1 , 1, &objectArray[1]}, {vec3(0,0,0),1 ,1 , &objectArray[2]}};
    std::cout << entityArray[0].getCoords().x << std::endl;
  /*  ObjEntity head(vec3(0,0,1),1, &objectArray[0]);
    ObjEntity fence(vec3(0,0,0),1, &objectArray[1]);
    ObjEntity board(vec3(0,0,0),1, &objectArray[2]);*/
	initOpenGLProgram(window); //Operacje inicjujące

	float angle_x = 0; //Kąt obrotu obiektu
	float angle_y = 0; //Kąt obrotu obiektu

	//glfwSetTime(0); //Wyzeruj licznik czasu
    frameTime = glfwGetTime();

	//Główna pętla
	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
	{
	    lastTime = glfwGetTime();
		angle_x += speed_x*deltaTime; //Zwiększ kąt o prędkość kątową razy czas jaki upłynął od poprzedniej klatki
		angle_y += speed_y*deltaTime; //Zwiększ kąt o prędkość kątową razy czas jaki upłynął od poprzedniej klatki

		drawScene(window,angle_x,angle_y, entityArray); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
        deltaTime = lastTime - glfwGetTime()  ;
        if(glfwGetTime()-frameTime  >= 1.0){
            frameTime = glfwGetTime();
            std::cout << "FPS: "<<framesCount <<std::endl;
            framesCount = 0;
		}
		else{
            framesCount++;
		}


	}

	freeOpenGLProgram();

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
