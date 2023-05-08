# include <iostream>
# include <stdio.h>
# include <stdlib.h>
# include <string>
# include <vector>
# include <sstream>
# include <iomanip>
# include <math.h>

using namespace std;

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

enum ErrorType {
  NO_MORE_INPUT,
  NO_CLOSING_QUOTE,
  UNRECOGNIZED_TOKEN,
  UNEXPECTED_TOKEN,
  UNDEFINEDID,
  DIVID_ZERO
} ;

enum PrimitiveType {
  CONSTRUCTOR,
  OPERATOR, 
  EQUIVALENCE
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

int g_uTestNum ;
int gLineNum ;
int gColumn ;

class SystemInfo {
public:

  static void ResetLnAndCol() {
    gLineNum = 1 ;
    gColumn =  0 ;
  } // ResetLnAndCol()

} ; // SystemInfo

class SystemFunctions {
public:

  static string To_string( char ch ) {
    stringstream ss ;
    ss << ch ;
    string str = ss.str() ;
    return str ;
  } // To_string()

  static bool IsDigit( char ch ) {
    return ( ch >= '0' && ch <= '9' ) ;
  } // IsDigit()

} ; // SystemFunctions

/**
 * Exception class is to deal with error type  
*/
class Exception {

public:
  ErrorType mErrorType_ ;
  string mCurrentToken_ ;

  Exception( ErrorType errorType ) {
    mErrorType_ = errorType ;
    mCurrentToken_ = "\0" ;
  } // Exception()

  Exception( ErrorType errorType, string& token ) {
    mErrorType_ = errorType ;
    mCurrentToken_ = token ;
  } // Exception()

} ; // Exception

class ErrorHadling {

private:

  static void NO_MORE_INPUT_ERROR() {
    cout << "ERROR (no more input) : END-OF-FILE encountered\n" ;
  } // NO_MORE_INPUT_ERROR()

  static void NO_CLOSING_QOUTE_ERROR() {
    cout << "ERROR (no closing quote) : " 
         << "END-OF-LINE encountered at Line " << gLineNum
         << " Column " << gColumn+1 << "\n" ; // gColumn+1 means it need next token to be ')'
  } // NO_CLOSING_QOUTE_ERROR()

  static void UNRECOGNIZED_TOKEN_ERROR( string token ) {
    cout << "Unrecognized token with first char : " << token << "\n" ;
  } // UNRECOGNIZED_TOKEN_ERROR()

  static void UNEXPECTED_TOKEN_ERROR( string token ) {
    cout << "ERROR (unexpected token) :" 
         << " atom or '(' expected when token at Line " << gLineNum
         << " Column " << gColumn
         << " is >>" << token << "<<\n" ;
  } // UNEXPECTED_TOKEN_ERROR()

  static void UNDEFINEDID_ERROR( string token ) {
    cout << "Undefined identifier :'" << token << "\n" ;
  } // UNDEFINEDID_ERROR()

  static void DIVID_ZERO_ERROR() {
    cout << "Devision by zero\n" ;
  } // DIVID_ZERO_ERROR()

public:

  static void ErrorMessage( ErrorType type, string token ) {
    if ( type == NO_MORE_INPUT ) NO_MORE_INPUT_ERROR() ;
    else if ( type == NO_CLOSING_QUOTE ) NO_CLOSING_QOUTE_ERROR() ;
    else if ( type == UNRECOGNIZED_TOKEN ) UNRECOGNIZED_TOKEN_ERROR( token ) ;
    else if ( type == UNEXPECTED_TOKEN ) UNEXPECTED_TOKEN_ERROR( token ) ;
    else if ( type == UNDEFINEDID ) UNDEFINEDID_ERROR( token ) ;
    else if ( type == DIVID_ZERO ) DIVID_ZERO_ERROR() ;
  } // ErrorMessage()

} ; // ErrorHadling

class Reader {

private:
  int mLineNum ;
  int mColumn ;
  char mCurrentChar ; // store current char user inputed

  void UpdateLineNumAndCol() {
    mColumn++ ;
    gColumn++ ;
    if ( mCurrentChar == '\n' ) {
      mLineNum++ ;
      mColumn = 1 ;
      // global
      gLineNum++ ;
      gColumn = 0 ;
    } // if

    // debug 
    // cout << " Ln: " << gLineNum << "  Col: " << gColumn << "  char: " << mCurrentChar <<  endl ;

  } // UpdateLineNumAndCol()

public:
  Reader() {
    mLineNum = 1 ;
    mColumn = 0 ;
    mCurrentChar = '\0' ;
  } // Reader()

  char PeekChar() {
    return cin.peek() ;
  } // PeekChar()

  char GetChar() {
    mCurrentChar = cin.get() ;
    UpdateLineNumAndCol() ;
    return mCurrentChar ;
  } // GetChar()

  char IsEnd() {
    return cin.peek() == EOF ;
  } // IsEnd()

  int GetLineNum() {
    return mLineNum ;
  } // GetLineNum()

  int GetColumn() {
    return mColumn ;
  } // GetColumn()

} ; // Reader

class Lexer {

private:

  Reader mReader_ ;
  TokenType mCurrent_tokenType_ ;
  string mCurrent_token_ ;
  int mLineNum_ ;
  int mColumn_ ;

  void UpdateTokenPos() {
    mLineNum_ = mReader_.GetLineNum() ;
    mColumn_ = mReader_.GetColumn() ;
  } // UpdateTokenPos()

  void Remove_RestOfCharInThisLine() {
    while ( mReader_.PeekChar() != '\n' ) {
      mReader_.GetChar() ;
    } // while
  } // Remove_RestOfCharInThisLine()

  char PreProcessingPeek() {
    char peekChar = mReader_.PeekChar() ;
    
    while ( IsWhiteSpace( peekChar ) || IsComment( peekChar ) || IsNewLine( peekChar ) ) {

      // comment case
      if ( IsComment( peekChar ) ) {
        Remove_RestOfCharInThisLine() ;
      } // if
      else {
        mReader_.GetChar() ;
      } // else

      peekChar = mReader_.PeekChar() ;
    } // while

    UpdateTokenPos() ; // update info of current token

    return peekChar ;
  } // PreProcessingPeek()

  string CutToken() {

    char peekChar = PreProcessingPeek() ;
    mCurrent_tokenType_ = SYMBOL ; // default token type

    if ( IsEOF( peekChar ) ) throw Exception( NO_MORE_INPUT ) ;

    if ( IsLeftParen( peekChar ) ) {
      mCurrent_tokenType_ = LEFT_PAREN ; // define the token type
      char ch = mReader_.GetChar() ;

      // if the token is (), meaning that token type actually is a nil
      if ( IsRightParen( PreProcessingPeek() ) ) {
        mReader_.GetChar() ; // remove the ) from input
        mColumn_ -= 1 ; // modifing the token position
        mCurrent_tokenType_ = NIL ; // define the token type
        return "nil" ;
      } // if : ()

      return SystemFunctions::To_string( ch ) ;
    } // if : (

    else if ( IsRightParen( peekChar ) ) {
      mCurrent_tokenType_ = RIGHT_PAREN ; // define the token type
      return SystemFunctions::To_string( mReader_.GetChar() ) ;
    } // else if : )

    else if ( IsDoubleQuote( peekChar ) ) {

      mCurrent_tokenType_ = STRING ; // define the token type
      return GetString() ;
    } // else if : "

    else if ( IsSingleQuote( peekChar ) ) {
      mCurrent_tokenType_ = QUOTE ; // define the token type
      return SystemFunctions::To_string( mReader_.GetChar() ) ;
    } // else if : '

    return CutRestOfToken() ; // if is a symbol, string, integer, float, then cut rest of them

  } // CutToken()

  string CutRestOfToken() {

    string buffer ;

    // continue reading until meeting separators
    while ( !IsSeparators( mReader_.PeekChar() ) ) {
      buffer.push_back( mReader_.GetChar() ) ;
    } // while

    return buffer ;
  } // CutRestOfToken()

  string GetString() {

    string buffer ; 
    buffer.push_back( mReader_.GetChar() ) ; // Get string starting token "

     
    char peekChar = mReader_.PeekChar() ; 

    while ( !IsDoubleQuote( peekChar ) ) {

      // check exception
      if ( peekChar == '\n' ) throw Exception( NO_CLOSING_QUOTE ) ;
      else if ( peekChar == EOF ) throw Exception( NO_MORE_INPUT ) ;
      
      char curChar = mReader_.GetChar() ;

      if ( curChar == '\\' && mReader_.PeekChar() == '\"' ) {
        buffer.push_back( curChar ) ;
        curChar = mReader_.GetChar() ;
      } // if : \"

      buffer.push_back( curChar ) ;
      peekChar = mReader_.PeekChar() ;
      // curChar = mReader_.GetChar() ;
    } // while

    buffer.push_back( mReader_.GetChar() ) ; // Get string ending token "
    return buffer ;
  } // GetString()

  // To check whether the char is separators
  bool IsSeparators( char ch ) {
    return ( ch == ' '  ||
             ch == '\t' ||
             ch == '\r' ||
             ch == '\n' ||
             ch == '('  ||
             ch == ')'  ||
             ch == '\"' ||
             ch == '\'' ||
             ch == '\0' ||
             ch == ';'  ) ;
  } // IsSeparators()

  // To check whether the char is left parent
  bool IsLeftParen( char ch ) {
    return ( ch == '(' ) ;
  } // IsLeftParen()

  // To check whether the char is right parent
  bool IsRightParen( char ch ) {
    return ( ch == ')' ) ;
  } // IsRightParen()

  // To check whether the char is double quote
  bool IsDoubleQuote( char ch ) {
    return ( ch == '\"' ) ;
  } // IsDoubleQuote()

  // To check whether the char is single quote
  bool IsSingleQuote( char ch ) {
    return ( ch == '\'' ) ;
  } // IsSingleQuote()

  // To check whether the char is white space
  bool IsWhiteSpace( char ch ) {
    return ( ch == ' '  ||
             ch == '\t' ||
             ch == '\r' ) ;
  } // IsWhiteSpace()

  // To check whether the token is newLine
  bool IsNewLine( char ch ) {
    return ( ch == '\n' ) ;
  } // IsNewLine()

  // To check whether the token is comment
  bool IsComment( char ch ) {
    return ( ch == ';' ) ;
  } // IsComment()

  bool IsDot( char ch ) {
    return ( ch == '.' ) ;
  } // IsDot()

  bool IsSign( char ch ) {
    return ( ch == '+' || ch == '-' ) ;
  } // IsSign()

  // To check whether the char is decimal
  bool IsDigit( char ch ) {
    return SystemFunctions::IsDigit( ch ) ;
  } // IsDigit()

  bool IsEOF( char ch ) {
    return ( ch == EOF ) ;
  } // IsEOF()

  // [sign]{digit}.[digit][f]
  bool IsFloatToken() {
    
    int pos = 0 ;
    int size = mCurrent_token_.size() ;

    // float size must bigger than two 
    if ( size < 2 ) return false ;
    else if ( size == 2 && IsSign( mCurrent_token_[0] ) && IsDot( mCurrent_token_[1] ) ) return false ;

    // check sign symbol
    if ( pos != size && IsSign( mCurrent_token_[pos] ) ) pos++ ;
    
    // 0~n times digit before Dot
    while ( pos != size && IsDigit( mCurrent_token_[pos] )  ) pos++ ;

    // The current char last in token doesn't Dot  
    if ( pos == size || !IsDot( mCurrent_token_[pos] ) ) return false ;
    pos++ ;

    // 0~n times digit after Dot
    while ( pos != size && IsDigit( mCurrent_token_[pos] ) ) pos++ ;
    
    // use 'f' symbol to end up
    if ( pos != size && mCurrent_token_[pos] == 'f' ) pos++ ;

    // if is end of token
    if ( pos == size ) return true ;

    return false ;
  } // IsFloatToken()

  bool IsDigitToken() {

    int pos = 0 ;
    int size = mCurrent_token_.size() ;

    // check sign symbol
    if ( pos != size && IsSign( mCurrent_token_[pos] ) ) pos++ ;

    // it the token is only a sign
    if ( pos == size ) return false ;

    // check each char of token
    for ( int i = pos ; i < mCurrent_token_.size() ; i++ ) {
      if ( !IsDigit( mCurrent_token_[i] ) ) return false ;
    } // for

    return true ;
  } // IsDigitToken()

  // To check the token type
  void CheckTokenAndType() {
    if ( mCurrent_token_ == "." ) {
      mCurrent_tokenType_ = DOT ;
    } // if
    else if ( mCurrent_token_ == "t" || mCurrent_token_ == "#t" ) {
      mCurrent_token_ = "#t" ;
      mCurrent_tokenType_ = T ;
    } // else if
    else if ( mCurrent_token_ == "#f" || mCurrent_token_ == "nil" ) {
      mCurrent_token_ = "nil" ;
      mCurrent_tokenType_ = NIL ;
    } // else if
    else if ( IsDigitToken() ) {
      mCurrent_tokenType_ = INT ;
    } // else if
    else if ( IsFloatToken() ) {
      mCurrent_tokenType_ = FLOAT ;
    } // else if

  } // CheckTokenAndType()

  Token CreateToken() {
    Token oneToken ;
    CheckTokenAndType() ; // Check token type

    oneToken.value = mCurrent_token_ ; 
    oneToken.type = mCurrent_tokenType_ ;
    oneToken.lineNum = mLineNum_ ;
    oneToken.column = mColumn_ ;
    return oneToken ;
  } // CreateToken()

public:

  Token GetToken() {
    mCurrent_token_ = CutToken() ; // get the string of token
    return CreateToken() ;
  } // GetToken()

  // If there are blanks or comment behide a S-Exp, clear it
  void ClearLineCommentAndBlanks() {

    char peekChar = mReader_.PeekChar() ;

    gLineNum = 1 ;
    gColumn = 0 ; // while read the first char, col will add 1 automatically

    while ( IsWhiteSpace( peekChar ) || IsComment( peekChar ) ) {

      // comment case go through encounter \n
      if ( IsComment( peekChar ) ) {
        Remove_RestOfCharInThisLine() ;
      } // if
      else {
        mReader_.GetChar() ;
      } // else

      peekChar = mReader_.PeekChar() ;
    } // while

    // Go to the new Line
    if ( IsNewLine( peekChar ) ) {
      mReader_.GetChar() ;
      gLineNum = 1 ; // reset Line Number
      gColumn = 0 ; // reset Column
    } // if
    // cout << gLineNum << gColumn ; // debug
  } // ClearLineCommentAndBlanks()

  void ClearRestOfCharInThisLine() {
    char peekChar = mReader_.PeekChar() ;
    while ( peekChar != EOF && peekChar != '\n' ) {
      mReader_.GetChar() ;
      peekChar = mReader_.PeekChar() ;
    } // while
  } // ClearRestOfCharInThisLine()

} ; // Lexer

class Parser {

private:
  Lexer mLexicalAnalyzer_ ;
  vector<Token> *mTokenList_ ;
  Token mCurToken ;

  void GetNextToken() {
    mCurToken = mLexicalAnalyzer_.GetToken() ;
    mTokenList_->push_back( mCurToken ) ;
  } // GetNextToken()

  // <S-exp> ::= <ATOM> 
  //         | LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN
  //         | QUOTE <S-exp>
  bool IsSExp() {

    // <ATOM>
    if ( IsAtom() ) {
      return true ;
    } // if

    // LEFT-PAREN <S-exp> { <S-exp> } [ DOT <S-exp> ] RIGHT-PAREN
    else if ( IsLeftParen() ) {
      GetNextToken() ;

      // <S-exp>
      if ( !IsSExp() ) return false ;
      GetNextToken() ;

      // { <S-exp> }
      while ( IsSExp() ) {
        GetNextToken() ;
      } // while

      // [ DOT <S-exp> ]
      if ( IsDot() ) {
        GetNextToken() ;
        if ( !IsSExp() ) return false ;
        GetNextToken() ;
      } // if

      // RIGHT-PAREN
      if ( !IsRightParen() ) return false ;

      return true ;
    } // else if

    // QUOTE <S-exp>
    else if ( IsQuote() ) {
      GetNextToken() ;
      return IsSExp() ;
    } // else if

    return false ;
  } // IsSExp()

  // <ATOM> ::= SYMBOL | INT | FLOAT | STRING | NIL | T | LEFT-PAREN RIGHT-PAREN
  bool IsAtom() {
    return ( mCurToken.type == SYMBOL ||
             mCurToken.type == INT    ||
             mCurToken.type == FLOAT  ||
             mCurToken.type == STRING ||
             mCurToken.type == NIL    ||
             mCurToken.type == T      ) ;
  } // IsAtom()

  // return true if current_token is (
  bool IsLeftParen() {
    return mCurToken.value == "(" ;
  } // IsLeftParen()

  // return true if current_token is )
  bool IsRightParen() {
    return mCurToken.value == ")" ;
  } // IsRightParen()

  // return true if current_token is '
  bool IsQuote() {
    return mCurToken.value == "\'" ;
  } // IsQuote()

  // return true if current_token is .
  bool IsDot() {
    return mCurToken.value == "." ;
  } // IsDot()

  void ClearTokenList() {
    mTokenList_->clear() ;
  } // ClearTokenList()

public:

  Parser() {
    mTokenList_ = new vector<Token>() ;
  } // Parser()

  void ClearLineCommentAndBlanks() {
    mLexicalAnalyzer_.ClearLineCommentAndBlanks() ;
  } // ClearLineCommentAndBlanks()

  void ClearRestOfCharInThisLine() {
    mLexicalAnalyzer_.ClearRestOfCharInThisLine() ;
  } // ClearRestOfCharInThisLine()

  // check whether the syntax of a S-Exp correct
  vector<Token>* SyntaxAnalyzing() {
    
    ClearTokenList() ;
    GetNextToken() ;

    if ( !IsSExp() ) throw Exception( UNEXPECTED_TOKEN, mCurToken.value ) ;

    ClearLineCommentAndBlanks() ;

    return mTokenList_ ;
  } // SyntaxAnalyzing()

} ; // Parser

class SemanticsAnalyzer {

} ; // SemanticsAnalyzer

class Evaluator {

private:

public:

  // Go through Right
  void Run( Node *root ) {

    Node *temp = root ;

    while ( temp->right != NULL ) {
      temp = temp->right ;
    } // while

  } // Run()

  bool IsExit( const Node *temp ) {

    if ( temp == NULL ) return false ;
    else if ( temp->content.type != LEFT_PAREN ) return false ;
    temp = temp->right ;

    if ( temp == NULL ) return false ;
    else if ( temp->content.value != "exit" ) return false ;
    temp = temp->right ;

    if ( temp == NULL ) return false ;
    else if ( temp->content.type != RIGHT_PAREN ) return false ;
    
    return true ;

  } // IsExit()

} ; // Evaluator

// Data structure
class Tree {

private:
  
  vector<Node*> *mRoots_ ;
  vector<Token> *mTokens_ ;

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

  bool IsAtom( Token* token ) {
    TokenType type = token->type ;
    return ( type != LEFT_PAREN &&
             type != RIGHT_PAREN && 
             type != DOT &&
             type != QUOTE ) ;
  } // IsAtom()

  Node* Build( vector<Token> *tokens ) {

    //  basic step
    if ( tokens->size() <= 0 ) {
      return NULL ;
    } // if
    
    Node* temp = CreatNewNode() ;
    temp->content = tokens->front() ;
    tokens->erase( tokens->begin() ) ;
    temp->right = Build( tokens ) ;

    return temp ;
  } // Build()

  void CreateNewRightNode( Node* &root, bool &goRight ) {
    goRight = false ;
    root->right = CreatNewNode() ;
    root->right->prevNode = root ;
    BuildTree( root->right, goRight ) ;
    root = root->right ;
  } // CreateNewRightNode()

  void CreateNewLeftNode( Node* &root, bool &goRight ) {
      root->left = CreatNewNode() ;
      root->left->prevNode = root ;
      BuildTree( root->left, goRight ) ;
      root = root->left ;
  } // CreateNewLeftNode()

  void BuildTree( Node* &root, bool &goRight ) {
      Token curToken = mTokens_->front() ; // Get first token
      mTokens_->erase( mTokens_->begin() ) ;
      
      while ( curToken.type == LEFT_PAREN || IsAtom( &curToken ) || curToken.type == QUOTE ) {
          CreateNewRightNode( root, goRight ) ;
          curToken = mTokens_->front();
          mTokens_->erase( mTokens_->begin() ) ;
      } // while

      if ( curToken.type == DOT ) {
          mTokens_->erase( mTokens_->begin() ) ;
          goRight = true;
          BuildTree( root, goRight ) ;
      } // if
      else if ( curToken.type == RIGHT_PAREN ) {
          return;
      } // else if

      // Place token in Node
      else if ( goRight ) {
          goRight = false;
          root->right->content = curToken ;
      } // else if
      else {
          root->left->content = curToken ;
      } // else

      if ( curToken.type == LEFT_PAREN ) {
          CreateNewLeftNode( root, goRight ) ;
          BuildTree( root, goRight ) ;
      } // if
  } // BuildTree

  // TODO
  // Node* BuildATree( Node* root, bool &goRight ) {

  //   // Terminal condition
  //   if ( curToken == NULL ) return NULL ;
  //   else if ( RIGHT_PAREN ) return NULL ;

  //   Node* temp = CreatNewNode() ;

  //   if ( curToken->content.type == LEFT_PAREN ) {
  //     temp->left = BuildATree( curToken->right ) ; // Go to next token
  //     // temp->content = curToken->content ;
  //   } // if

  //   else if ( IsAtom( curToken ) ) {
  //     temp->left = BuildATree( curToken->right ) ;
  //     temp->content = curToken->content ;
  //   } // else if

  //   else if ( curToken->content.type == DOT ) {
  //     curToken = curToken->right ; // go next token
  //     if ( IsAtom( curToken ) ) {
  //       temp->right = BuildATree( curToken->right ) ;
  //       temp->content = curToken->content ;
  //     } // if
  //     else if ( curToken->content.type == RIGHT_PAREN ) {
  //       temp->right = BuildATree( curToken->right ) ;
  //     } // else if
  //   } // else if

  //   return temp ;
  // } // BuildATree()

  // TODO
  void InorderTraversal( Node* root ) {
    if ( root == NULL ) {
        return ;
    } // if
    InorderTraversal(root->left);
    cout << root->content.value << " " ;
    InorderTraversal(root->right);
  } // InorderTraversal()

  // Print string to consol
  void PrintString( const Token& strToken ) {

    const string STR = strToken.value ;
    const int SIZE = STR.size() ;

    for ( int i = 0 ; i < SIZE ; i++ ) {
      char curChar = STR[i] ;

      if ( curChar == '\\' && ( i+1 ) < SIZE ) {
        char newChar = STR[++i] ;

        if ( newChar == 'n' ) {
          cout << "\n" ;
        } // if \n
        
        else if ( newChar == 't' ) {
          cout << "\t" ;
        } // else if \t
        
        else if ( newChar == '\"' ) {
          cout << "\"" ;
        } // else if \"

        else if ( newChar == '\\' ) {
          cout << "\\" ;
        } // else if \"

        else {
          cout << curChar << STR[i] ;
        } // else
      } // if
      else {
        cout << curChar ;
      } // else
    } // for

    cout << endl ;
  } // PrintString()

  void PrintFloat( const Token& pointToken ) {
    
    string buffer = pointToken.value ;

    // Sign bit
    if ( buffer[0] == '+' ) {
      buffer.erase( buffer.begin() ) ;
    } // if
    else if ( buffer[0] == '-' ) {
      cout << '-' ;
      buffer.erase( buffer.begin() ) ;
    } // else if

    cout << fixed << setprecision( 3 )
         << round( atof( buffer.c_str() )*1000 )/1000
         << endl ;

  } // PrintFloat()

  void PrintInt( const Token& strToken ) {
    const string INTEGER = strToken.value ;
    int startingPos = 0 ;

    if ( INTEGER[startingPos] == '+' ) startingPos++ ;

    for ( int i = startingPos ; i < INTEGER.size() ; i++ ) {
      cout << INTEGER[i] ;
    } // for

    cout << endl ;

  } // PrintInt()

public:
  
  Tree() {
    mRoots_ = new vector<Node*>() ;
    mTokens_ = new vector<Token>() ;
  } // Tree()

  void PrintNodes( Node *head ) {

    Node *temp = head ;
    
    while ( temp != NULL ) {
      if ( temp->content.type == STRING ) PrintString( temp->content ) ;
      else if ( temp->content.type == FLOAT ) PrintFloat( temp->content ) ;
      else if ( temp->content.type == INT ) PrintInt( temp->content ) ;
      else cout << temp->content.value << endl ;
      temp = temp->right ;
    } // while

  } // PrintNodes()

  // TODO
  // void PrettyPrint( Node *head ) {

  //   Node *temp = head ;
    
  //   while ( temp != NULL ) {
  //     if ( temp->content.type == STRING ) PrintString( temp->content ) ;
  //     else if ( temp->content.type == FLOAT ) PrintFloat( temp->content ) ;
  //     else if ( temp->content.type == INT ) PrintInt( temp->content ) ;
  //     else cout << temp->content.value << endl ;
  //     temp = temp->right ;
  //   } // while

  // } // PrettyPrint()

  void BuildTree( vector<Token> *tokens ) {
    mTokens_ = tokens ;
  } // BuildTree()

  void BuildASequentialTree( vector<Token> *tokens ) {

    mTokens_ = tokens ;

    Node *root = NULL ;
    root = Build( tokens ) ; // recurrence
    mRoots_->push_back( root ) ; // add a new tree root
  } // BuildASequentialTree()

  Node *GetCurrentRoot() {
    if ( !mRoots_->empty() ) return mRoots_->back() ;
    return NULL ;
  } // GetCurrentRoot()

} ; // Tree

// OurSheme main system
class OurSheme {

private:

  Parser mSyntaxAnalyzer_ ;
  SemanticsAnalyzer mSemanticsAnalyzer_ ;
  Tree mAtomTree_ ;
  Evaluator mEvaluator_ ;

  vector<vector<Token>*> *mTokenTable_ ; // saving primitive tokens until building tree
  bool mQuit_ ;

  void PrintTokenList() {
    for ( int i = 0 ; i < mTokenTable_->size() ; i++ ) {
      cout << mTokenTable_->back()->at( i ).value << endl ;
    } // for
  } // PrintTokenList()

  void ReadSExp() {
    vector<Token>* tokenList = mSyntaxAnalyzer_.SyntaxAnalyzing() ;
    if ( !tokenList->empty() ) {
      mTokenTable_->push_back( tokenList ) ;
      mAtomTree_.BuildASequentialTree( tokenList ) ;
      tokenList->clear() ;
    } // if 
  } // ReadSExp()

public:

  OurSheme() {
    mTokenTable_ = new vector<vector<Token>*>() ;
    mQuit_ = false ;
  } // OurSheme()

  void Run() {

    cout << "Welcome to OurScheme!" << endl << endl ;
    
    while ( !mQuit_ ) {

      cout << "> " ;
      
      try {
        ReadSExp() ;
        Node *curAtomRoot = mAtomTree_.GetCurrentRoot() ;
        if ( mEvaluator_.IsExit( curAtomRoot ) ) mQuit_ = true ;
        else mAtomTree_.PrintNodes( curAtomRoot ) ;

      } // try
      catch ( const Exception& e ) {
        
        ErrorHadling::ErrorMessage( e.mErrorType_, e.mCurrentToken_ ) ;
        if ( e.mErrorType_ == NO_MORE_INPUT ) mQuit_ = true ;
        else mSyntaxAnalyzer_.ClearRestOfCharInThisLine() ;

        // if ( g_uTestNum == 1 )  mQuit_ = true ; // debug

        gLineNum = 1 ;
        gColumn = 0 ; // while read the first char, col will add 1 automatically
      } // catch

      if ( !mQuit_ ) cout << endl ;
    } // while

    cout << "Thanks for using OurScheme!" << endl << endl ;

  } // Run()

} ; // OurSheme

int main() {

  cin >> g_uTestNum ; // PL testNum
  gLineNum = 1 ;
  gColumn = 0 ;

  

  OurSheme ourSheme = OurSheme() ;
  ourSheme.Run() ;

  return 0 ;
} // main()