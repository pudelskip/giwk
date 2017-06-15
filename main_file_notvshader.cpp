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

vec3 jedzenie;
vec3 przesun[121];
wolne_pola wolne[121];
int pole[11][11];
bool glodny=true;

//Uchwyty na shadery
ShaderProgram *shaderProgram;
ShaderProgram *shaderProgram1; //Wskaźnik na obiekt reprezentujący program cieniujący.

//Uchwyty na VAO i bufory wierzchołków
GLuint vao;
GLuint bufVertices; //Uchwyt na bufor VBO przechowujący tablicę współrzędnych wierzchołków
GLuint bufColors;  //Uchwyt na bufor VBO przechowujący tablicę kolorów
GLuint bufNormals;

GLuint vao_plan;
GLuint bufVertices_plan;


 //Uchwyt na bufor VBO przechowujący tablickę wektorów normalnych

//Kostka
float* vertices=Models::CubeInternal::vertices;
float* colors=Models::CubeInternal::colors;//
float* normals=Models::CubeInternal::normals;
int vertexCount=Models::CubeInternal::vertexCount;

float ground[]={
    11.0f,-1.0f,11.0f,1.0f,
    -11.0f,-1.0f,11.0f,1.0f,
    11.0f,-1.0f,-11.0f,1.0f,

    -11.0f,-1.0f,-11.0f,1.0f,
    11.0f,-1.0f,-11.0f,1.0f,
    -11.0f,-1.0f,11.0f,1.0f
    };

    glm::mat4 obrot= glm::mat4(1.0f);

//Czajnik
//float* vertices=Models::TeapotInternal::vertices;
//float* colors=Models::TeapotInternal::colors;
//float* normals=Models::TeapotInternal::vertexNormals;
//int vertexCount=Models::TeapotInternal::vertexCount;




//float przesun[4] = {5.0f,5.0f,0.0f,0.0f};


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

	//*****Przygotowanie do rysowania pojedynczego obiektu*******
	//Zbuduj VBO z danymi obiektu do narysowania
	bufVertices=makeBuffer(vertices, vertexCount, sizeof(float)*4); //VBO ze współrzędnymi wierzchołków
	bufColors=makeBuffer(colors, vertexCount, sizeof(float)*4);//VBO z kolorami wierzchołków
	bufNormals=makeBuffer(normals, vertexCount, sizeof(float)*4);//VBO z wektorami normalnymi wierzchołków
    bufVertices_plan= makeBuffer(ground, 6, sizeof(float)*4);


	//Zbuduj VAO wiążący atrybuty z konkretnymi VBO
	glGenVertexArrays(1,&vao); //Wygeneruj uchwyt na VAO i zapisz go do zmiennej globalnej
    glGenVertexArrays(1,&vao_plan);


	glBindVertexArray(vao); //Uaktywnij nowo utworzony VAO

	assignVBOtoAttribute(shaderProgram,"vertex",bufVertices,4); //"vertex" odnosi się do deklaracji "in vec4 vertex;" w vertex shaderze
	assignVBOtoAttribute(shaderProgram,"color",bufColors,4); //"color" odnosi się do deklaracji "in vec4 color;" w vertex shaderze
	assignVBOtoAttribute(shaderProgram,"normal",bufNormals,4);
	//"normal" odnosi się do deklaracji "in vec4 normal;" w vertex shaderze

	glBindVertexArray(0);

	glBindVertexArray(vao_plan); //Uaktywnij nowo utworzony VAO

	assignVBOtoAttribute(shaderProgram1,"vertices",bufVertices_plan,4);

	glBindVertexArray(0);
	//Dezaktywuj VAO
	//******Koniec przygotowania obiektu************

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

void drawObject(GLuint vao, ShaderProgram *shaderProgram, mat4 mP, mat4 mV, mat4 mM) {
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

//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window,float mov1,float mov2,bool moved, int n) {
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
	glm::mat4 V1 = glm::lookAt( //Wylicz macierz widoku
		glm::vec3(0.0f, 25.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, -1.0f));




	//Wylicz macierz modelu rysowanego obiektu
	glm::mat4 M = glm::mat4(1.0f);

	//M = glm::translate(M,przesun[0]);

    //drawObject(vao,shaderProgram,P,V,M);


	for(int i=0;i<n;i++){
        glm::mat4 M = glm::mat4(1.0f);
        glm:mat4 Move = glm::mat4(1.0f);
        Move = Move*obrot;
        Move[3].x=przesun[i].x;
        Move[3].z=przesun[i].z;
        M=Move;
        //M = glm::translate(M,przesun[i]);

        drawObject(vao,shaderProgram,P,V,M);

	}

    if(glodny){
       glm::mat4 M = glm::mat4(1.0f);
        M = glm::translate(M,jedzenie);

        drawObject(vao,shaderProgram,P,V,M);

    }
	//M = glm::rotate(M, angle_x, glm::vec3(1, 0, 0));
	//M = glm::rotate(M, angle_y, glm::vec3(0, 1, 0));

	//Narysuj obiekt
	//drawObject(vao,shaderProgram,P,V,M);

	//Przerzuć tylny bufor na przedni

	//rysoawnie ziemi


	M = glm::mat4(1.0f);
     drawObject2(vao_plan,shaderProgram1,P,V,M);


	glfwSwapBuffers(window);

}



int main(void)
{

    int n=3;
    int wylosowana;
	GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów

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

    przesun[0]=vec3(0.0f,0.0f,0.0f);
    przesun[1]=vec3(0.0f,0.0f,2.0f);
    przesun[2]=vec3(0.0f,0.0f,4.0f);

    obrot[0].x=0;
    obrot[0].z=-1;
    obrot[2].x=1;
    obrot[2].z=0;



    jedzenie = vec3(0.0f,0.0f,-6.0f);

    for(int i=0;i<11;i++)
        for(int j=0;j<11;j++)
            pole[i][j]=0;


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


	//	mov1 +=speed1*glfwGetTime();
	//	mov2 +=speed2*glfwGetTime();
        bool moved=false;
		//Zwiększ kąt o prędkość kątową razy czas jaki upłynął od poprzedniej klatki

        if(cam_angle>20) cam_angle=20;
        if(cam_angle<15) cam_angle=15;
        czas += glfwGetTime();
        cam_angle+=cam_speed*glfwGetTime();
        glfwSetTime(0);

		if(czas>0.9f)
        {
            czas=0;
            //glfwSetTime(0); //Wyzeruj licznik czasu
            mov1+=speed1*2;
            mov2+=speed2*2;





            for(int i=n;i>0;i--){
                przesun[i]=przesun[i-1];
            }



            przesun[0].z=mov2;
            przesun[0].x=mov1;




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
                n+=1;


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

            }

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

		drawScene(window,mov1,mov2,moved,n); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.

	}

	freeOpenGLProgram();

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
