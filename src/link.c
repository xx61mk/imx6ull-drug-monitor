#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/link.h"

Link_t *create_link(void)
{
    Link_t *plink = (Link_t *)malloc(sizeof(Link_t));
    if (NULL == plink)
    {
        return NULL;
    }
    plink->phead = NULL;
    plink->clen = 0;
    return plink;
}

int insert_link_head(Link_t *plink, LData_t data)
{
    if (NULL == plink)
    {
        return -1;
    }
    Node_t *pnew = (Node_t *)malloc(sizeof(Node_t));
    if (NULL == pnew)
    {
        return -1;
    }
    pnew->data = data;
    pnew->pnext = plink->phead;
    plink->phead = pnew;
    plink->clen++;
    return 0;
}

int insert_link_tail(Link_t *plink, LData_t data)
{
    if (NULL == plink)
    {
        return -1;
    }
    Node_t *pnew = (Node_t *)malloc(sizeof(Node_t));
    if (NULL == pnew)
    {
        return -1;
    }
    pnew->data = data;
    pnew->pnext = NULL;

    if (NULL == plink->phead)
    {
        plink->phead = pnew;
    }
    else
    {
        Node_t *ptmp = plink->phead;
        while (ptmp->pnext != NULL)
        {
            ptmp = ptmp->pnext;
        }
        ptmp->pnext = pnew;
    }
    plink->clen++;
    return 0;
}

int delete_link_head(Link_t *plink)
{
    if (NULL == plink || NULL == plink->phead)
    {
        return -1;
    }
    Node_t *ptmp = plink->phead;
    plink->phead = ptmp->pnext;
    free(ptmp);
    plink->clen--;
    return 0;
}

int delete_link_tail(Link_t *plink)
{
    if (NULL == plink || NULL == plink->phead)
    {
        return -1;
    }
    if (NULL == plink->phead->pnext)
    {
        free(plink->phead);
        plink->phead = NULL;
        plink->clen = 0;
        return 0;
    }
    Node_t *ptmp = plink->phead;
    while (ptmp->pnext->pnext != NULL)
    {
        ptmp = ptmp->pnext;
    }
    free(ptmp->pnext);
    ptmp->pnext = NULL;
    plink->clen--;
    return 0;
}

void show_link(Link_t *plink)
{
    if (NULL == plink)
    {
        return;
    }
    printf("[link] %d thread(s):", plink->clen);
    Node_t *ptmp = plink->phead;
    while (ptmp != NULL)
    {
        printf(" %s", ptmp->data.name);
        ptmp = ptmp->pnext;
    }
    printf("\n");
}

void destroy_link(Link_t *plink)
{
    if (NULL == plink)
    {
        return;
    }
    Node_t *ptmp = plink->phead;
    while (ptmp != NULL)
    {
        Node_t *pnext = ptmp->pnext;
        free(ptmp);
        ptmp = pnext;
    }
    free(plink);
}
