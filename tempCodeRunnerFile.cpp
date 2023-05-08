#include <iostream>

using namespace std ;

enum TokenType {
  LEFT_PAREN = 0,
  RIGHT_PAREN = 1, 
  INT = 2,
  STRING = 3,
  DOT = 4,
  FLOAT = 5,
  NIL = 6,
  T = 7,
  QUOTE = 8,
  SYMBOL = 9
} ;

struct Token {
  string value ;
  TokenType type ;
  int lineNum ;
  int column ;
} ;

struct Node {
  Token content ;
  Node *left ;
  Node *right ;
  Node *prevNode ;
} ;

bool IsAtom( Node* node ) {
  TokenType type = node->content.type ;
  return ( type != LEFT_PAREN &&
            type != RIGHT_PAREN && 
            type != DOT &&
            type != QUOTE ) ;
} // IsAtom()

// create and initialize new Atom
Node* CreatNewNode() {
  Node *oneNode = new Node() ;

  oneNode->left = NULL ;
  oneNode->right = NULL ;
  oneNode->prevNode = NULL ;

  return oneNode ;
} // CreatNewNode()

// create and initialize new Node
Node* CreatNewNode( Token token ) {
  Node *oneNode = new Node() ;

  oneNode->content = token ;
  oneNode->left = NULL ;
  oneNode->right = NULL ;
  oneNode->prevNode = NULL ;

  return oneNode ;
} // CreatNewNode()

Node* BuildATree( Node* curToken ) {

  // Terminal condition
  if ( curToken == NULL || curToken->content.type == RIGHT_PAREN ) return NULL ;

  Node* temp = CreatNewNode() ;

  if ( curToken->content.type == LEFT_PAREN ) {
    temp->left = BuildATree( curToken->right ) ; // Go to next token
    // temp->content = curToken->content ;
  } // if

  else if ( IsAtom( curToken ) ) {
    temp->left = BuildATree( curToken->right ) ;
    temp->content = curToken->content ;
  } // else if

  else if ( curToken->content.type == DOT ) {
    curToken = curToken->right ; // go next token
    if ( IsAtom( curToken ) ) {
      temp->right = BuildATree( curToken->right ) ;
      temp->content = curToken->content ;
    } // if
    else if ( curToken->content.type == LEFT_PAREN ) {
      temp->right = BuildATree( curToken->right ) ;
    } // else if
  } // else if

  return temp ;
} // BuildATree()

void InorderTraversal( Node* root ) {
  if ( root == NULL ) {
      return ;
  } // if
  InorderTraversal( root->left );
  cout << root->content.value << " " ;
  InorderTraversal( root->right );
} // InorderTraversal()

int main() {

  Token T1 ;
  Token T2 ;
  Token T3 ;
  Token T4 ;

  T1.value = "(" ;
  T1.type = LEFT_PAREN ;

  T2.value = "1" ;
  T2.type = INT ;

  T3.value = "2" ;
  T3.type = INT ;

  T4.value = ")" ;
  T4.type = RIGHT_PAREN ;

  Node* root = NULL ;

  Node* N1 = CreatNewNode(T1) ;
  Node* N2 = CreatNewNode(T2) ;
  Node* N3 = CreatNewNode(T3) ;
  Node* N4 = CreatNewNode(T4) ;

  root->right = N1 ;
  N1->right = N2 ;
  N2->right = N3 ;
  N3->right = N4 ;

  Node* treeRoot = BuildATree( root ) ;
  InorderTraversal( treeRoot ) ;

  return 0 ;
}