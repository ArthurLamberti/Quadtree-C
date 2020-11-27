#include "quadtree.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h> /* OpenGL functions */
#endif

#include <math.h>
unsigned int first = 1;
char desenhaBorda = 1;

QuadNode *newNode(int x, int y, int width, int height)
{
    QuadNode *n = malloc(sizeof(QuadNode));
    n->x = x;
    n->y = y;
    n->width = width;
    n->height = height;
    n->NW = n->NE = n->SW = n->SE = NULL;
    n->color[0] = n->color[1] = n->color[2] = 0;
    n->id = first++;
    return n;
}

//ajeitar o nivel de detalhamento
QuadNode *geraNodoRecursivo(int x, int y, int width, int height, Img *pic, float minDetail, int nivelDetail){

    QuadNode *novaRaiz = newNode(x,y,width,height);
    if(nivelDetail > (int)minDetail){
        return novaRaiz;
    }
    RGB(*pixels)
    [pic->width] = (RGB(*)[pic->width])pic->img;

    int rMedio, gMedio, bMedio, difPixelMedio = 0;
    /*
        calcular a cor media da regiao (rgb medio / height * width) 
    */

    //calcula o rgb medio
    for (int i = y; i < y + height; i++)
    {
        for (int j = x; j < x + width; j++)
        {
            rMedio += pixels[i][j].r;
            gMedio += pixels[i][j].g;
            bMedio += pixels[i][j].b;
        }
    }
    //verifica se a altura * largura eh diferente de zero para calcular a media
    if(height * width != 0){
        rMedio = rMedio / (height * width);
        gMedio = gMedio / (height * width);
        bMedio = bMedio / (height * width);
    }

    
    //usar formula disponivel no moodle para todos os pixels
    for (int i = y; i < y + height; i++)
    {
        for (int j = x; j < x + width; j++)
        {
            difPixelMedio += sqrt(pow((pixels[i][j].r - rMedio), 2) + pow((pixels[i][j].g - gMedio), 2) + pow((pixels[i][j].b - bMedio), 2));
        }
    }

    if(height * width != 0){
        difPixelMedio = difPixelMedio / (height * width);
    }
    
    novaRaiz->color[0] = rMedio;
    novaRaiz->color[1] = gMedio;
    novaRaiz->color[2] = bMedio;

    //se diferenca media for maior do que o minDetail, chama recursivamente o geraNodoRecursivo
    if (difPixelMedio > minDetail)
    {
        novaRaiz->status = PARCIAL;

        novaRaiz->NE = geraNodoRecursivo(x, y, width / 2, height / 2, pic, minDetail, nivelDetail+1);
        novaRaiz->NW = geraNodoRecursivo(x + (width / 2), y, width / 2, height / 2, pic, minDetail, nivelDetail+1);
        novaRaiz->SE = geraNodoRecursivo(x, y + (height / 2), width / 2, height / 2, pic, minDetail, nivelDetail+1);
        novaRaiz->SW = geraNodoRecursivo(x + (width / 2), y + (height / 2), width / 2, height / 2, pic, minDetail, nivelDetail+1);
    }
    else
    {
        novaRaiz->status = CHEIO;
    }
    return novaRaiz;
}

QuadNode *geraQuadtree(Img *pic, float minDetail)
{


    int width = pic->width;
    int height = pic->height;

    QuadNode *raiz = geraNodoRecursivo(0,0,width,height,pic, minDetail, 0);

    return raiz;
}

// Limpa a memória ocupada pela árvore
void clearTree(QuadNode *n)
{
    if (n == NULL)
        return;
    if (n->status == PARCIAL)
    {
        clearTree(n->NE);
        clearTree(n->NW);
        clearTree(n->SE);
        clearTree(n->SW);
    }
    //printf("Liberando... %d - %.2f %.2f %.2f %.2f\n", n->status, n->x, n->y, n->width, n->height);
    free(n);
}

// Ativa/desativa o desenho das bordas de cada região
void toggleBorder()
{
    desenhaBorda = !desenhaBorda;
    printf("Desenhando borda: %s\n", desenhaBorda ? "SIM" : "NÃO");
}

// Desenha toda a quadtree
void drawTree(QuadNode *raiz)
{
    if (raiz != NULL)
        drawNode(raiz);
}

// Grava a árvore no formato do Graphviz
void writeTree(QuadNode *raiz)
{
    FILE *fp = fopen("quad.dot", "w");
    fprintf(fp, "digraph quadtree {\n");
    if (raiz != NULL)
        writeNode(fp, raiz);
    fprintf(fp, "}\n");
    fclose(fp);
    printf("\nFim!\n");
}

void writeNode(FILE *fp, QuadNode *n)
{
    if (n == NULL)
        return;

    if (n->NE != NULL)
        fprintf(fp, "%d -> %d;\n", n->id, n->NE->id);
    if (n->NW != NULL)
        fprintf(fp, "%d -> %d;\n", n->id, n->NW->id);
    if (n->SE != NULL)
        fprintf(fp, "%d -> %d;\n", n->id, n->SE->id);
    if (n->SW != NULL)
        fprintf(fp, "%d -> %d;\n", n->id, n->SW->id);
    writeNode(fp, n->NE);
    writeNode(fp, n->NW);
    writeNode(fp, n->SE);
    writeNode(fp, n->SW);
}

// Desenha todos os nodos da quadtree, recursivamente
void drawNode(QuadNode *n)
{
    if (n == NULL)
        return;

    glLineWidth(0.1);

    if (n->status == CHEIO)
    {
        glBegin(GL_QUADS);
        glColor3ubv(n->color);
        glVertex2f(n->x, n->y);
        glVertex2f(n->x + n->width - 1, n->y);
        glVertex2f(n->x + n->width - 1, n->y + n->height - 1);
        glVertex2f(n->x, n->y + n->height - 1);
        glEnd();
    }

    else if (n->status == PARCIAL)
    {
        if (desenhaBorda)
        {
            glBegin(GL_LINE_LOOP);
            glColor3ubv(n->color);
            glVertex2f(n->x, n->y);
            glVertex2f(n->x + n->width - 1, n->y);
            glVertex2f(n->x + n->width - 1, n->y + n->height - 1);
            glVertex2f(n->x, n->y + n->height - 1);
            glEnd();
        }
        drawNode(n->NE);
        drawNode(n->NW);
        drawNode(n->SE);
        drawNode(n->SW);
    }
    // Nodos vazios não precisam ser desenhados... nem armazenados!
}
