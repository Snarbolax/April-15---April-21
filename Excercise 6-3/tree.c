//
//  tree.c
//  tree
//
//  Created by William McCarthy on 134//20.
//  Copyright © 2020 William McCarthy. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "tree.h"
#include "utils.h"

int line = 1;

char* strdup(const char *s)   /* make a duplicate of s */
{
	char *p;
	p = (char *)malloc(strlen(s) + 1);  /* +1 for ′\0′ */
	if (p != NULL)
		strcpy(p, s);
	return p;
}

//-------------------------------------------------
tnode* tnode_create(const char* word, int l) {
  tnode* p = (tnode*)malloc(sizeof(tnode));
  p->word = strdup(word);    // copy of word allocated on heap
  p->count = 1;
  p->lines[0] = l;
  p->left = NULL;
  p->right = NULL;
  for (int i = 1; i < BUFSIZ; ++i) { p->lines[i] = 0; }

  return p;
}

//====================================================================
void tnode_delete(tnode* p) {
    free((void*)p->word);
    free(p);
}

//====================================================================
tree* tree_create(void) {
  tree* p = (tree*)malloc(sizeof(tree));
  p->root = NULL;
  p->size = 0;
  
  return p;
}

//====================================================================
static void tree_deletenodes(tree* t, tnode* p) {
  if (p == NULL) { return; }
  
  tree_deletenodes(t, p->left);
  tree_deletenodes(t, p->right);
  tnode_delete(p);
  t->size--;
}

//====================================================================
void tree_clear(tree* t) {
  tree_delete(t);
  t->root = NULL;
  t->size = 0;
}

//====================================================================
void tree_delete(tree* t) { tree_deletenodes(t, t->root); }

//====================================================================
bool tree_empty(tree* t)  { return t->size == 0; }

//====================================================================
size_t tree_size(tree* t) { return t->size; }

//====================================================================
static tnode* tree_addnode(tree* t, tnode** p, const char* word, int l) {
  int compare;
  int i;
  
  if (*p == NULL) {
    *p = tnode_create(word, l);
  } else if ((compare = strcmp(word, (*p)->word)) == 0) {
    (*p)->count++;
	for (i = 1; p->lines[i] && i < BUFSIZ ; ++i) { ; }
	p->lines[i] = l;
  } else if (compare < 0) { tree_addnode(t, &(*p)->left, word, l);
  } else {
    tree_addnode(t, &(*p)->right, word, l);
  }

  return *p;
}

//====================================================================
static char* str_process(char* s, char* t) {
  char* p = s;
  char ignore[] = "\'\".,;;?!()/’";
  while (*t != '\0') {
    if (strchr(ignore, *t) == NULL || (*t == '\'' && (p != s || p != s + strlen(s) - 1))) {
      *p++ = tolower(*t);
    }
    ++t;
  }
  *p++ = '\0';
  return s;
}

//====================================================================
tnode* tree_add(tree* t, char* word, int l) {
  char buf[100];
  
  if (word == NULL) { return NULL; }
  str_process(buf, word);
  
  tnode* p = tree_addnode(t, &(t->root), buf, l);
  t->size++;

  return p;
}

//====================================================================
static void tree_printme(tree* t, tnode* p) {
  if (p->count > 1) { printf("%5d -- ", p->count); }
  else {
    printf("         ");
  }
  printf("%-18s -- ", p->word);
  for (int i = 0; i < BUFSIZE && p->lines[i]; ++i) { printf("%s%d", i == 0 ? "" : ", ", p->lines[i]); }
  printf("\n");
}

//====================================================================
static void tree_printnodes(tree* t, tnode* p) {
  if (p == NULL) { return; }
  
  tree_printnodes(t, p->left);
  tree_printme(t, p);
  tree_printnodes(t, p->right);
}

//====================================================================
static void tree_printnodes_preorder(tree* t, tnode* p) {
  if (p == NULL) { return; }
  
  tree_printme(t, p);
  tree_printnodes(t, p->left);
  tree_printnodes(t, p->right);
}

//====================================================================
static void tree_printnodes_postorder(tree* t, tnode* p) {
  if (p == NULL) { return; }
  
  tree_printnodes_postorder(t, p->left);
  tree_printnodes_postorder(t, p->right);
  tree_printme(t, p);
}

//====================================================================
static void tree_printnodes_reverseorder(tree* t, tnode* p) {
  if (p == NULL) { return; }
  
  tree_printnodes_reverseorder(t, p->right);
  tree_printme(t, p);
  tree_printnodes_reverseorder(t, p->left);
}

//====================================================================
//void tree_print_levelorder(tree* t);

//====================================================================
void tree_print(tree* t)              { tree_printnodes(t, t->root);               printf("\n"); }

//====================================================================
void tree_print_preorder(tree* t)     { tree_printnodes_preorder(t, t->root);      printf("\n"); }

//====================================================================
void tree_print_postorder(tree* t)    { tree_printnodes_postorder(t, t->root);     printf("\n"); }

//====================================================================
void tree_print_reverseorder(tree* t) { tree_printnodes_reverseorder(t, t->root);  printf("\n"); }

//====================================================================
void tree_test(tree* t) {
  printf("=============== TREE TEST =================================\n");
  printf("\n\nprinting in order...========================================\n");
  tree_print(t);
  printf("end of printing in order...=====================================\n\n");

  printf("\n\nprinting in reverse order...================================\n");
  tree_print_reverseorder(t);
  printf("end of printing in reverse order...=============================\n\n");
  printf("tree size is: %zu\n", tree_size(t));
  
  printf("clearing tree...\n");
  tree_clear(t);
  printf("after clearing tree, size is: %zu\n", tree_size(t));
  tree_print(t);
  
  printf("=============== END OF TREE TEST ==========================\n");
}

//====================================================================
tree* tree_from_file(int argc, const char* argv[]) {
  if (argc != 2) { return NULL; }

  FILE* fin;
  const char* filename = argv[1];
  if ((fin = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "Could not open file: '%s'\n", filename);
    exit(1);
  }

  char buf[BUFSIZ];
  char delims[] = " \n";
  int size = 0;
  memset(buf, 0, sizeof(buf));

  tree* t = tree_create();
  while (fgets(buf, BUFSIZ, fin)) {
    char* word = strtok(buf, delims);
    tree_add(t, word, line);

    while ((word = strtok(NULL, delims)) != NULL) {
      tree_add(t, word, line);
    }
  }
  printf("%d words added...\n", size);
  fclose(fin);

  return t;
}

int noise(char* word)
{
	 char *nw[] = { "the", "and", "an", "a", "are", "for", "in", "is", "it", "of", "on", "or", "that", "this", "to", "was" };

	int cond;
	int l = 0;
	int m;
	int h;

	if (word[1] == '\0') { return 1; }
	h = sizeof(nw) / sizeof(char *) - 1;
	while (l <= h) {
		m = (l + h) / 2;
		if ((cond = strcmp(word, nw[mid])) < 0) { h = m - 1; }
		else if (cond > 0) { l = m + 1; }
		else { return 1; }
	}

	return 0;
}

//====================================================================
int main(int argc, const char* argv[]) {
  tree* t = tree_from_file(argc, argv);
  if (t != NULL) { tree_test(t);  tree_delete(t);  return 0; }
  
  char buf[BUFSIZ];
  char delims[] = " .,;?!\'\"()\n";
  int size = 0;
  memset(buf, 0, sizeof(buf));
  
  in = fopen(argv[1], "r");

  t = tree_create();
  while (fgets(buf, BUFSIZ, stdin)) {	
    char* word = strtok(buf, delims);
	if (!isnoise(word)) { tree_add(t, word, line); }
	else if (word[0] == '\n') { line++; }

    while ((word = strtok(NULL, delims)) != NULL) {
		if (!isnoise(word)) { tree_add(t, word, line); }
		else if (word[0] == '\n') { line++; }
    }
  }
  printf("%d words added...\n", size);

  //tree_test(t);
  tree_delete(t);
  
  return 0;
}

