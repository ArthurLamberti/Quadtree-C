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

/*      Description: vai gerando nodos recursivamente de acordo com nivel de detalhamento
    *   Returns: QuadNode 
    *   Parameters: x: ponto x do nodo, 
    *               y: ponto y do nodo, 
    *               width: largura do nodo, 
    *               height: altura do nodo, 
    *               minDetail: detalhe passado do usuario, 
    *               nivelDetail: nivel do detalhamento atual 
    * arredondar calculo para considerar todos os pixels
*/
QuadNode *geraNodoRecursivo(int x, int y, int width, int height, Img *pic, float minDetail)
{

    QuadNode *novaRaiz = newNode(x, y, width, height);

    RGB(*pixels)
    [pic->width] = (RGB(*)[pic->width])pic->img;

    int rMedio, gMedio, bMedio, nivelDeDetalhe = 0;
    int areaDaImagem = width * height;
    // printf("%d - ", areaDaImagem);
    //deve somar todos os pixels da area
    for (int i = y; i < height + y; i++)
    {
        for (int j = x; j < width + x; j++)
        {
            rMedio += pixels[i][j].r;
            gMedio += pixels[i][j].g;
            bMedio += pixels[i][j].b;
        }
    }

    //verifica pra nao dar divisao por 0 e faz o calculo do rbg medio

    if (areaDaImagem != 0)
    {
        rMedio = rMedio / areaDaImagem;
        gMedio = gMedio / areaDaImagem;
        bMedio = bMedio / areaDaImagem;
    }
    //atribui a raiz a cor media
    novaRaiz->color[0] = rMedio;
    novaRaiz->color[1] = gMedio;
    novaRaiz->color[2] = bMedio;

    //usa formula pra calcular o nivel de detalhe
    int rDistancia, gDistancia, bDistancia = 0;
    for (int i = y; i < height + y; i++)
    {
        for (int j = x; j < width + x; j++)
        {
            //obtem as potencias internas do calculo
            // rDistancia = pow((pixels[i][j].r - rMedio), 2);
            // gDistancia = pow((pixels[i][j].g - gMedio), 2);
            // bDistancia = pow((pixels[i][j].b - bMedio), 2);
            // nivelDeDetalhe += sqrt(rDistancia + gDistancia + bDistancia);
            nivelDeDetalhe += sqrt(pow((pixels[i][j].r - rMedio), 2) + pow((pixels[i][j].g - gMedio), 2) + pow((pixels[i][j].b - bMedio), 2));
        }
    }
    //verifica se a area da imagem eh igual a zero pra nao dividir com erro
    if (areaDaImagem != 0)
    {
        nivelDeDetalhe = nivelDeDetalhe / areaDaImagem;
    }

    //deve arredondar a imagem corretamente (se for numero impar, passar um pixel a mais na imagem da direita) // C arredonda para baixo
    //deve chamar recursivamente a funcao para cada area
    // printf("\n%d  === %f\n", nivelDeDetalhe, minDetail);
    if (nivelDeDetalhe > minDetail)
    {
        novaRaiz->status = PARCIAL;

        // novaRaiz->NE = geraNodoRecursivo(x, y, (int)round(width / 2), (int)round(height / 2), pic, minDetail);
        // novaRaiz->NW = geraNodoRecursivo(x + (width / 2), y, (int)ceil(width / 2), (int)ceil(height / 2), pic, minDetail);
        // novaRaiz->SW = geraNodoRecursivo(x + (width / 2), y + (height / 2), (int)ceil(width / 2), (int)ceil(height / 2), pic, minDetail);
        // novaRaiz->SE = geraNodoRecursivo(x, y + (height / 2), (int)round(width / 2), (int)round(height / 2), pic, minDetail);

        novaRaiz->NE = geraNodoRecursivo(x, y, round(width / 2), round(height / 2), pic, minDetail);
        novaRaiz->NW = geraNodoRecursivo(x + (width / 2), y, round(width / 2), round(height / 2), pic, minDetail);
        novaRaiz->SW = geraNodoRecursivo(x + (width / 2), y + (height / 2), round(width / 2), round(height / 2), pic, minDetail);
        novaRaiz->SE = geraNodoRecursivo(x, y + (height / 2), round(width / 2), round(height / 2), pic, minDetail);
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
    QuadNode *raiz = geraNodoRecursivo(0, 0, width, height, pic, minDetail);

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
        glVertex2f(n->x + n->width, n->y);
        glVertex2f(n->x + n->width, n->y + n->height);
        glVertex2f(n->x, n->y + n->height);
        glEnd();
    }

    else if (n->status == PARCIAL)
    {
        drawNode(n->NE);
        drawNode(n->NW);
        drawNode(n->SE);
        drawNode(n->SW);
    }
    if (desenhaBorda)
    {
        glBegin(GL_LINE_LOOP);
        glColor3ub(0, 0, 0);
        glVertex2f(n->x, n->y);
        glVertex2f(n->x + n->width, n->y);
        glVertex2f(n->x + n->width, n->y + n->height);
        glVertex2f(n->x, n->y + n->height);
        glEnd();
    }
    // Nodos vazios não precisam ser desenhados... nem armazenados!
}