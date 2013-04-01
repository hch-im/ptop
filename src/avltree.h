/*
 * avltree.h
 *
 *  Created on: Jan 31, 2010
 *      Author: hchen
 */

#ifndef AVLTREE_H_
#define AVLTREE_H_
#include "common.h"

typedef int ElementType;
typedef int PCFile;

struct AvlNode;
typedef struct AvlNode * Position;
typedef struct AvlNode * AvlTree;

struct AvlNode
{
	ElementType Element;
	PCFile	fd;
	AvlTree  Left;
	AvlTree  Right;
	int      Height;
	struct pid_stats* pst_list[2];
};

AvlTree empty( AvlTree T );
Position find( ElementType X, AvlTree T );
void insert( ElementType X, PCFile fd, AvlTree* T ,AvlTree* np);
AvlTree delete( ElementType X, AvlTree T );
void clear_position(Position position);
void sfree_pid(Position p);
#endif /* AVLTREE_H_ */
