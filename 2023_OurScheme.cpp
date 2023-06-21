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
  Token *content ;
  Node *left ;
  Node *right ;
  int subroutineNum ;
  bool visited ;
  bool isCons ;
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


// ---------------------------Reader ------------------------------------ //

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


// ---------------------------Lexical Analyzation------------------------ //

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
    
    if ( peekChar == '\n' ) {
      mReader_.GetChar() ;
    } // if 
  } // ClearRestOfCharInThisLine()

} ; // Lexer


// ---------------------------Syntax Analyzation------------------------- //

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


// ---------------------------Semantics Analyzation---------------------- //

class SemanticsAnalyzer {

} ; // SemanticsAnalyzer


// ---------------------------Evaluator---------------------------------- //

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

    if ( temp == NULL || temp->content == NULL ) return false ;
    else if ( ! ( temp->isCons ) ) return false ;
    else if ( temp->content->value != "exit" ) return false ;
    return true ;

  } // IsExit()

} ; // Evaluator

class Printer {
  
private: 

  int mCurSubroutine_ ;
  int mPrintLeftBracketCount_ ;
  bool mIsPared_ ;
  bool mIsConsNeedPrintRightBracket ;

  // Print String to consol
  void PrintString( const Token* strToken ) {

    const string STR = strToken->value ;
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

  // Print Float to consol
  void PrintFloat( const Token* pointToken ) {
    
    string buffer = pointToken->value ;

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

  // Print Int to consol
  void PrintInt( const Token* strToken ) {
    const string INTEGER = strToken->value ;
    int startingPos = 0 ;

    if ( INTEGER[startingPos] == '+' ) startingPos++ ;

    for ( int i = startingPos ; i < INTEGER.size() ; i++ ) {
      cout << INTEGER[i] ;
    } // for

    cout << endl ;

  } // PrintInt()

  // Check whether the DS is ( 1 . 2 ) format
  // Paired only has on right node event thought is cons
  bool IsPaired( Node *head ) {
    return ( head->content == NULL && 
             head->left != NULL    && 
             head->right != NULL ) ;
  } // IsPaired()

  // Check whether the DS is a cons
  // Cons have one or more left nodes
  bool IsCons( Node *head ) {
    return ( head->left != NULL ) ;
  } // IsCons()

  bool IsBoolean( Node *head ) {
    return ( head->content->type == NIL || head->content->type == T ) ;
  } // IsBoolean()

  string CombineLeftBracket() {
    string buffer ;
    while ( mPrintLeftBracketCount_ > 0 ) {
      buffer += "( " ; 
      mPrintLeftBracketCount_-- ;
    } // while

    return buffer ;
  } // CombineLeftBracket()

  void PrintSpaceseLeftBracket() {
    // cout << "subroutine: " << " (" << mCurSubroutine_ << ") " << endl ; // DEBUG
    if ( mPrintLeftBracketCount_ > 0 ) {
      cout << right << setw( mCurSubroutine_*2 ) << CombineLeftBracket() ;
    } // if
    else {
      cout << right << setw( mCurSubroutine_*2 ) << " " ;
    } // else

  } // PrintSpaceseLeftBracket()

  void PrintRightBracket() {
    if ( mCurSubroutine_ > 0 ) cout << right << setw( mCurSubroutine_*2 ) << " " ;
    cout << ")" << endl ;
  } // PrintRightBracket()

  void ConsBegin() {
    mPrintLeftBracketCount_++ ;
    mCurSubroutine_++ ;
  } // ConsBegin()

  void ConsEnd() {
    mCurSubroutine_-- ;
    PrintRightBracket() ;
  } // ConsEnd()

  void PrintNodeToken( Node *temp ) {
    if ( temp->content->type == STRING ) PrintString( temp->content ) ;
    else if ( temp->content->type == FLOAT ) PrintFloat( temp->content ) ;
    else if ( temp->content->type == INT ) PrintInt( temp->content ) ;
    else if ( temp->content->type == NIL && mCurSubroutine_ > 0 ) return ;
    else cout << temp->content->value << endl ;
  } // PrintNodeToken()

public:
  
  void PrettyPrintNodes( Node *head ) {
    
    Node *cur = head ;
    bool isPared = false ;

    // Pared Case
    if ( IsPaired( cur ) ) {
      
      bool onlyOneRightNode = false ;
      bool leftCons_RightPared = false ;

      // check whether pared node's right has only one node ( A . B )
      if ( cur->right->left == NULL ) onlyOneRightNode = true ;

      // if ( ( A B ) . ( C . D ) )
      if ( cur->left->content  != NULL &&
           cur->left->left     != NULL &&
           cur->right->content == NULL ) leftCons_RightPared = true ;

      // if right node is pared ( A . ( B . C ) )
      if ( cur->right->content == NULL ) cur->right->visited = true ;

      // ---------- start print node ---------- //
      if ( ! cur->visited ) ConsBegin() ;
      // ---------- go left node ---------- //
      /* 
      if left node is cons and right node is pared, 
      in order to separate left node and right's pared's left node,
      we need to add bracket
      */
      if ( leftCons_RightPared ) {
        
        ConsBegin() ;
        PrettyPrintNodes( cur->left ) ;
        ConsEnd() ;
        
      } // if
      else PrettyPrintNodes( cur->left ) ;


      // if only one right node such as ( A . B ) print dot
      if ( onlyOneRightNode && !IsBoolean( cur->right ) ) {
        PrintSpaceseLeftBracket() ;
        cout << "." << endl ;
      } // if

      // ---------- go right node ---------- //
      PrettyPrintNodes( cur->right ) ;

      if ( ! cur->visited ) ConsEnd() ;

    } // if: Pared
    // Cons Case
    else if ( cur->content != NULL ) {

      PrintSpaceseLeftBracket() ;
      PrintNodeToken( cur ) ;

      // ---------- go left node ---------- //
      if ( cur->left != NULL ) PrettyPrintNodes( cur->left ) ;

    } // else if

  } // PrettyPrintNodes()

  void PrettyPrint( Node *head ) {
    mPrintLeftBracketCount_ = 0 ;
    mCurSubroutine_ = 0 ;
    mIsPared_ = false ;
    mIsConsNeedPrintRightBracket = false ;

    if ( ! head->isCons ) PrintNodeToken( head ) ;
    else {
      if ( head->content != NULL ) ConsBegin() ; // if root not a pared
      PrettyPrintNodes( head ) ;
      if ( mCurSubroutine_ > 0 ) ConsEnd() ;
    } // else: Print a tree

  } // PrettyPrint()

  // used to debug
  void PrintConstruct( Node *head ) {
    Node *cur = head ;
    if ( cur == NULL ) return ;
    if ( cur->content != NULL ) {
      cout << cur->subroutineNum << " " ;
      PrintNodeToken( cur ) ;
    } // if
    else cout << cur->subroutineNum << " _" << endl ;


    PrintConstruct( cur->left ) ;
    PrintConstruct( cur->right ) ;

  } // PrintConstruct()

  void PrintVector( vector<Token> *tokenList ) {
    for ( int i = 0 ; i < tokenList->size() ; i++ ) {
      cout << tokenList->at(i).value << " " ;
    } // for
    cout << endl ;
  } // PrintVector()

} ; // Printer

// ---------------------------Tree Data Structure------------------------ //

class Tree {

private:
  
  vector<Node*> *mRoots_ ;
  vector<Token> *mTokensQueue_ ;

  Token mCurToken_ ;
  int mCurSubroutine_ ;
  
  // create and initialize new Atom
  Node* CreateANewNode() {
    Node *oneNode = new Node() ;

    oneNode->left = NULL ;
    oneNode->right = NULL ;
    oneNode->content = NULL ;
    oneNode->subroutineNum = mCurSubroutine_ ;
    oneNode->visited = false ;
    oneNode->isCons = true ;

    return oneNode ;
  } // CreateANewNode()

  // Check whether Token is atom 
  bool IsAtom( Token token ) {
    TokenType type = token.type ;
    return ( type != LEFT_PAREN &&
             type != RIGHT_PAREN && 
             type != DOT &&
             type != QUOTE ) ;
  } // IsAtom()

  // Push token vector to queue
  void SetTokenList( vector<Token>* tokenList ) {
    mTokensQueue_ = tokenList ;
  } // SetTokenList()

  // pop out the first token in token list
  Token GetNextToken() {
    mCurToken_ = mTokensQueue_->front() ; // Get first token
    mTokensQueue_->erase( mTokensQueue_->begin() ) ;
    return mCurToken_ ;
  } // GetNextToken()

  // Peek next token in token list
  Token PeekNextToken() {
    return mTokensQueue_->front() ; // Get first token ;
  } // PeekNextToken()

  // Copy current token to a new Token
  Token* CopyCurToken() {
    Token* newToken = new Token() ;
    newToken->column = mCurToken_.column ;
    newToken->lineNum = mCurToken_.lineNum ;
    newToken->type = mCurToken_.type ;
    newToken->value = mCurToken_.value ;
    return newToken ;
  } // CopyCurToken()

  Node* SaveToken_with_NewNode() {
    Node* temp = CreateANewNode() ;
    temp->content = CopyCurToken() ;
    cout << "Save Token: " << " (" << temp->subroutineNum << ") " 
         << temp->content->value << endl ; // DEBUG 
    return temp ;
  } // SaveToken_with_NewNode()

  // Build the cons structure
  Node* BuildCons() {
    
    mCurSubroutine_++ ;
    
    Node* root = CreateANewNode() ;
    Node* cur = root ;

    // buildind a sub tree
    if ( mCurToken_.type == LEFT_PAREN ) {
      GetNextToken() ;

      // case1: ( A B C ) save left atom to node
      // case2: ( A ( B C ) D ) if meet the left bracket, create new node to connect it
      while ( IsAtom( mCurToken_ ) || mCurToken_.type == LEFT_PAREN ) {

        // if is atom svae to left node
        if ( IsAtom( mCurToken_ ) ) cur->left = SaveToken_with_NewNode() ;
        else if ( mCurToken_.type == LEFT_PAREN ) cur->left = BuildCons() ;

        // get the next token
        GetNextToken() ;

        // if the got next token is a dot, cur can't move to right node
        // cause the right node need to check 
        if ( mCurToken_.type != DOT ) {  
          cur->right = CreateANewNode() ;
          cur = cur->right ;
        } // if
        
      } // while

      if ( mCurToken_.type == DOT ) {
        GetNextToken() ;

        if ( IsAtom( mCurToken_ ) ) cur->right = SaveToken_with_NewNode() ;
        else if ( mCurToken_.type == LEFT_PAREN ) cur->right = BuildCons() ;
        GetNextToken() ;

      } // if: Dot

      if ( mCurToken_.type == RIGHT_PAREN ) {
        mCurSubroutine_-- ;
        return root ;
      } // if

    } // if

    return root ;

  } // BuildCons()

  // As the input is not a cons structure 
  Node* BuildOnlyOneNode() {
    Node *root = CreateANewNode() ;
    root->content = CopyCurToken() ;
    root->isCons = false ;
    return root ;
  } // BuildOnlyOneNode()

public:
  
  Tree() {
    mRoots_ = new vector<Node*>() ;
    mTokensQueue_ = new vector<Token>() ;
    mCurSubroutine_ = 0 ;
  } // Tree()

  // Create A New Tree For Current Token List
  void CreatANewTree( vector<Token>* tokenList ) {

    // ---------- Initialize ---------- //
    SetTokenList( tokenList ) ; // push tokenList in 
    mCurSubroutine_ = 0 ;

    // null node in root
    Node* treeRoot = CreateANewNode() ;
    GetNextToken() ;

    // ---------- Building A Tree ---------- //
    if ( tokenList->empty() ) treeRoot = BuildOnlyOneNode() ;
    else treeRoot = BuildCons() ;

    // ---------- Finish ---------- //
    mRoots_->push_back( treeRoot ) ;
  } // CreatANewTree()

  // return current tree root
  Node *GetCurrentRoot() {
    if ( !mRoots_->empty() ) return mRoots_->back() ;
    return NULL ;
  } // GetCurrentRoot()

} ; // Tree

// ---------------------------OurScheme System--------------------------- //

// OurScheme main system
class OurScheme {

private:

  Parser mSyntaxAnalyzer_ ;
  SemanticsAnalyzer mSemanticsAnalyzer_ ;
  Tree mAtomTree_ ;
  Evaluator mEvaluator_ ;
  Printer mPrinter_ ;

  vector<vector<Token>*> *mTokenTable_ ; // saving primitive tokens until building tree
  bool mQuit_ ;
  bool mErrorQuit_ ;

  void PrintTokenList() {
    for ( int i = 0 ; i < mTokenTable_->size() ; i++ ) {
      cout << mTokenTable_->back()->at( i ).value << endl ;
    } // for
  } // PrintTokenList()

  void ReadSExp() {
    vector<Token>* tokenList = mSyntaxAnalyzer_.SyntaxAnalyzing() ;

    if ( !tokenList->empty() ) {
      mTokenTable_->push_back( tokenList ) ; // push token list into token table to record

      // Building A Tree
      mAtomTree_.CreatANewTree( tokenList ) ;
      
      // Clear Tokens
      tokenList->clear() ;
    } // if 
  } // ReadSExp()

public:

  OurScheme() {
    mTokenTable_ = new vector<vector<Token>*>() ;
    mQuit_ = false ;
    mErrorQuit_ = false ;
  } // OurScheme()

  void Run() {

    cout << "Welcome to OurScheme!" << endl << endl ;
    
    while ( !mQuit_ && !mErrorQuit_ ) {

      cout << "> " ;
      
      try {
        ReadSExp() ;
        Node *curAtomRoot = mAtomTree_.GetCurrentRoot() ;
        if ( mEvaluator_.IsExit( curAtomRoot ) ) mQuit_ = true ;
        else mPrinter_.PrintConstruct( curAtomRoot ) ; // Pretty print
      } // try
      catch ( const Exception& e ) {
        
        ErrorHadling::ErrorMessage( e.mErrorType_, e.mCurrentToken_ ) ;
        if ( e.mErrorType_ == NO_MORE_INPUT ) mErrorQuit_ = true ;
        else mSyntaxAnalyzer_.ClearRestOfCharInThisLine() ;
        
        gLineNum = 1 ;
        gColumn = 0 ; // while read the first char, col will add 1 automatically
      } // catch

      if ( !mErrorQuit_ ) cout << endl ;
    } // while

    cout << "Thanks for using OurScheme!" << endl << endl ;

  } // Run()

} ; // OurSheme




// ---------------------------Main Function------------------------------ //

int main() {

  // cin >> g_uTestNum ; // PL testNum
  // cin.get() ;

  gLineNum = 1 ;
  gColumn = 0 ;
  

  OurScheme ourSheme = OurScheme() ;
  ourSheme.Run() ;

  return 0 ;
} // main()
