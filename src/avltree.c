/*
 * avltree.c
 *
 *  Created on: Jan 31, 2010
 *      Author: hchen
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "avltree.h"
#include "eperf.h"
#include "common.h"

void salloc_pid(Position p);
void sfree_pid(Position p);

AvlTree empty( AvlTree T )
{
	if( T != NULL )
	{
		empty( T->Left );
		empty( T->Right );
		//close performance counter
		if(T->fd > 0)
			close(T->fd);
		sfree_pid(T);
		free( T );
	}
	return NULL;
}

Position find( ElementType X, AvlTree T )
{
	if( T == NULL )
		return NULL;
	if( X < T->Element )
		return find( X, T->Left );
	else
		if( X > T->Element )
			return find( X, T->Right );
		else
			return T;
}

static int height( Position P )
{
	if( P == NULL )
		return -1;
	else
		return P->Height;
}

static int Max( int Lhs, int Rhs )
{
	return Lhs > Rhs ? Lhs : Rhs;
}

static Position SingleRotateWithLeft( Position K2 )
{
	Position K1;

	K1 = K2->Left;
	K2->Left = K1->Right;
	K1->Right = K2;

	K2->Height = Max( height( K2->Left ), height( K2->Right ) ) + 1;
	K1->Height = Max( height( K1->Left ), K2->Height ) + 1;

	return K1;  /* New root */
}


static Position SingleRotateWithRight( Position K1 )
{
	Position K2;

	K2 = K1->Right;
	K1->Right = K2->Left;
	K2->Left = K1;

	K1->Height = Max( height( K1->Left ), height( K1->Right ) ) + 1;
	K2->Height = Max( height( K2->Right ), K1->Height ) + 1;

	return K2;  /* New root */
}

static Position DoubleRotateWithLeft( Position K3 )
{
	/* Rotate between K1 and K2 */
	K3->Left = SingleRotateWithRight( K3->Left );

	/* Rotate between K3 and K2 */
	return SingleRotateWithLeft( K3 );
}

static Position DoubleRotateWithRight( Position K1 )
{
	/* Rotate between K3 and K2 */
	K1->Right = SingleRotateWithLeft( K1->Right );

	/* Rotate between K1 and K2 */
	return SingleRotateWithRight( K1 );
}

void insert( ElementType X, PCFile fd, AvlTree* root ,AvlTree* np)
{
	if((*root) == NULL)
	{
		/* Create and return a one-node tree */
		(*root) = (AvlTree)malloc( sizeof( struct AvlNode ) );
		memset(*root, 0, sizeof(struct AvlNode));
		(*np) = (*root);
		if( (*root) == NULL ){
			fprintf( stderr, "%s\n", "fail to create node." );
			return;
		}else
		{
			(*root)->Element = X; (*root)->Height = 0;
			(*root)->Left = (*root)->Right = NULL;
			(*root)->fd = fd;
			salloc_pid((*root));
		}
	}
	else
		if( X < (*root)->Element )
		{
			insert( X, fd, &((*root)->Left), np);
			if( height( (*root)->Left ) - height( (*root)->Right ) == 2 ){
				if( X < (*root)->Left->Element )
					(*root) = SingleRotateWithLeft( (*root) );
				else
					(*root) = DoubleRotateWithLeft( (*root) );
			}
		}
		else if( X > (*root)->Element )
		{
			insert( X, fd, &((*root)->Right) , np);
			if( height((*root)->Right ) - height( (*root)->Left ) == 2 ){
				if( X > (*root)->Right->Element )
					(*root) = SingleRotateWithRight( (*root) );
				else
					(*root) = DoubleRotateWithRight( (*root) );
			}
		}
	/* Else X is in the tree already; we'll do nothing */

	(*root)->Height = Max( height( (*root)->Left ), height( (*root)->Right ) ) + 1;
}

AvlTree delete( ElementType X, AvlTree T )
{
	//TODO implement this method
	return T;
}

void clear_position(Position position)
{
	if(position->fd > 0)
		close(position->fd);
	position->fd = -1;
}

void salloc_pid(Position p)
{
	int i;

	for (i = 0; i < 2; i++) {
		if ((p->pst_list[i] = (struct pid_stats *) malloc(PID_STATS_SIZE)) == NULL) {
			perror("malloc");
			exit(4);
		}
		memset(p->pst_list[i], 0, PID_STATS_SIZE);
	}
}

void sfree_pid(Position p)
{
	int i;

	for (i = 0; i < 2; i++) {
		if (p->pst_list[i]) {
			free(p->pst_list[i]);
			p->pst_list[i] = NULL;
		}
	}
}
