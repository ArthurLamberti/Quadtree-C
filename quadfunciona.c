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

QuadNode *fillNode(Img *pic, int x, int y, int width, int height, float minDetail)
{
    QuadNode *raiz = newNode(x, y, width, height);

    RGB(*pixels)
    [pic->width] = (RGB(*)[pic->width])pic->img;

    int rMediano, gMediano, bMediano, difMediaTodosPixels = 0;
    //gera a diferenca mediana dos pixels

    for (int i = y; i < y + height; i++)
    {
        for (int j = x; j < x + width; j++)
        {
            rMediano += pixels[i][j].r;
            gMediano += pixels[i][j].g;
            bMediano += pixels[i][j].b;
        }
    }

    if ((width * height) != 0)
    {
        rMediano = rMediano / (width * height);
        gMediano = gMediano / (width * height);
        bMediano = bMediano / (width * height);
    }

    for (int i = y; i < y + height; i++)
    {
        for (int j = x; j < x + width; j++)
        {
            difMediaTodosPixels += sqrt(pow((pixels[i][j].r - rMediano), 2) + pow((pixels[i][j].g - gMediano), 2) + pow((pixels[i][j].b - bMediano), 2));
        }
    }

    if ((width * height) != 0)
        difMediaTodosPixels = difMediaTodosPixels / (width * height);

    raiz->color[0] = rMediano;
    raiz->color[1] = gMediano;
    raiz->color[2] = bMediano;

    if (difMediaTodosPixels > minDetail)
    {
        raiz->status = PARCIAL;

        raiz->NE = fillNode(pic, x, y, width / 2, height / 2, minDetail);
        raiz->NW = fillNode(pic, x + (width / 2), y, width / 2, height / 2, minDetail);
        raiz->SE = fillNode(pic, x, y + (height / 2), width / 2, height / 2, minDetail);
        raiz->SW = fillNode(pic, x + (width / 2), y + (height / 2), width / 2, height / 2, minDetail);
    }
    else
    {
        raiz->status = CHEIO;
    }

    return raiz;
}

QuadNode *geraQuadtree(Img *pic, float minDetail)
{
    int width = pic->width;
    int height = pic->height;

    //////////////////////////////////////////////////////////////////////////
    // Implemente aqui o algoritmo que gera a quadtree, retornando o nodo raiz
    //////////////////////////////////////////////////////////////////////////

    QuadNode *raiz = fillNode(pic, 0, 0, width, height, minDetail);

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
