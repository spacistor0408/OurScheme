# include <iostream>
# include <stdio.h>
# include <stdlib.h>
# include <string>
# include <vector>
# include <sstream>
# include <iomanip>
# include <math.h>

using namespace std;

#define MAX_MEMORY_SIZE 1024

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
  UNEXPECTED_RIGHTBRACKET_TOKEN,
  UNDEFINEDID,
  DIVID_ZERO,
  MEMORY_NOT_ENOUGH,
  UNBOUND_SYMBOL,
  NON_LIST,
  NON_FUNCTION,
  LEVEL_OF_EXIT,
  LEVEL_OF_DEFINE,
  LEVEL_OF_CLEAN_ENVIRONMENT,
  INCORRECT_ARGUMENTS,
  INCORRECT_DEFINE_FORMAT,
  INCORRECT_ARGUMENT_TYPE,
  TESTING
  
} ;

enum PrimitiveType {
  CONSTRUCTOR,          // cons    (2), list   (>=0)
  QUOTE_BYPASS,         // '       (1), quote  (1)
  DEFINE,               // define  (2)
  PART_ACCESSORS,       // car     (1), cdr    (1)
  PRIMITIVE_PREDICTATE, // atom? pair? list? null? integer? real? number? string? boolean? symbol? (1)
  BASIC_ARITHMETIC,     // not (1) + - * / and or > >= < <= = string-append string>? string<? string=? (>=2)
  EQUIVALENCE,          // eqv?    (2), equal? (2)
  BEGIN,                // begin   (>=1)
  CONDITIONAL,          // if      (2 or 3), cond (>=1)
  CLEAN_ENVIRONMENT,    // clean-environment(0)
  EXIT,
  NONE
} ;

struct Token {
  string value ;
  TokenType type ;
  PrimitiveType primitiveType ;
} ;

struct Node {
  Token *content ;
  Node *left ;
  Node *right ;
  int subroutineNum ;
  bool isConsBegin ;
} ;

// Type of Data in Symbol table 
struct Symbol {
  int subroutine ;
  string name ;
  Node* binding ; // record the tree root 
  Symbol* next ;
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

  static string To_string( float ch ) {
    stringstream ss ;
    ss << ch ;
    string str = ss.str() ;
    return str ;
  } // To_string()

  static float To_float( string str ) {
    return stof( str ) ;
  } // To_float()

  static bool IsDigit( char ch ) {
    return ( ch >= '0' && ch <= '9' ) ;
  } // IsDigit()

  static Token CreateNewToken( string value, TokenType type ) {
    Token newToken ;
    newToken.value = value ;
    newToken.type = type ;
    newToken.primitiveType = NONE ;
    return newToken ; 
  } // CreateNewToken()

} ; // SystemFunctions



// ---------------------------Printer---------------------------------- //

class Printer {
  
private: 

  static int sCurSubroutine_ ;
  static int sPrintLeftBracketCount_ ;

  // Print String to consol
  static void PrintString( const Token* strToken ) {

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
  static void PrintFloat( const Token* pointToken ) {
    
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
  static void PrintInt( const Token* strToken ) {
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
  static bool IsPaired( Node *head ) {
    return ( head->content        == NULL && 
             head->left           != NULL && 
             head->right          != NULL && 
             head->right->content != NULL &&
             head->right->content->type != NIL ) ;
  } // IsPaired()

  static bool IsBoolean( Node *head ) {
    return ( head->content->type == NIL || head->content->type == T ) ;
  } // IsBoolean()

  static string CombineLeftBracket() {
    string buffer ;
    while ( sPrintLeftBracketCount_ > 0 ) {
      buffer += "( " ; 
      sPrintLeftBracketCount_-- ;
    } // while

    return buffer ;
  } // CombineLeftBracket()

  static void SetFormat() {
    // cout << "subroutine: " << " (" << sCurSubroutine_ << ") " << endl ; // DEBUG
    if ( sPrintLeftBracketCount_ > 0 ) {
      cout << right << setw( sCurSubroutine_*2 ) << CombineLeftBracket() ;
    } // if
    else if ( sCurSubroutine_ > 0 ) {
      cout << right << setw( sCurSubroutine_*2 ) << " " ;
    } // else if

  } // SetFormat()

  static void PrintRightBracket() {
    if ( sCurSubroutine_ > 0 ) cout << right << setw( sCurSubroutine_*2 ) << " " ;
    cout << ")" << endl ;
  } // PrintRightBracket()

  static void ConsBegin() {
    sPrintLeftBracketCount_++ ;
    sCurSubroutine_++ ;
  } // ConsBegin()

  static void ConsEnd() {
    sCurSubroutine_-- ;
    PrintRightBracket() ;
  } // ConsEnd()

  static void PrintNodeToken( Node *temp ) {
    if ( temp->content->type == STRING ) PrintString( temp->content ) ;
    else if ( temp->content->type == FLOAT ) PrintFloat( temp->content ) ;
    else if ( temp->content->type == INT ) PrintInt( temp->content ) ;
    else if ( temp->content->type == QUOTE ) cout << "quote" << endl ;
    else cout << temp->content->value << endl ;
  } // PrintNodeToken()

  // Traversal and print all node
  static void TraversalTree( Node *head, bool isCons ) {
    
    Node *cur = head ;

    // terminal node
    if ( cur == NULL ) return ;
    
    // ---------- case1: empty null ---------- //
    if ( cur->content == NULL ) {

      // ---------- start print node ---------- //
      if ( isCons ) ConsBegin() ; // add a ( to print queue
      // ---------- go left node ---------- //
      TraversalTree( cur->left, true ) ;

      // if is paired ( A . B )
      if ( IsPaired( cur ) ) {
        SetFormat() ;
        cout << "." << endl ;
      } // if

      // ---------- go right node ---------- //
      TraversalTree( cur->right, false ) ;
      if ( isCons ) ConsEnd() ;

    } // if

    // ---------- case2: right node is nil ---------- //
    else if ( cur->content->type == NIL && !isCons ) {
      return ;
    } // else if
    
    // ---------- case3: atom ---------- //
    else {
      SetFormat() ;
      PrintNodeToken( cur ) ;

    } // else

  } // TraversalTree()

public:

  static void PrettyPrint( Node *head ) {
    sPrintLeftBracketCount_ = 0 ;
    sCurSubroutine_ = 0 ;

    // Traversal
    TraversalTree( head, true ) ;

  } // PrettyPrint()

  // used to debug
  static void PrintConstruct( Node *head ) {
    Node *cur = head ;
    if ( cur == NULL ) {
      cout << "null" << endl ;
      return ;
    } // if

    if ( cur->content != NULL ) {
      cout << "  " ;
      PrintNodeToken( cur ) ;
    } // if
    else cout << " _" << endl ;

    PrintConstruct( cur->left ) ;
    PrintConstruct( cur->right ) ;

  } // PrintConstruct()

  static void PrintVector( vector<Token> *tokenList ) {
    for ( int i = 0 ; i < tokenList->size() ; i++ ) {
      cout << tokenList->at( i ).value << " " ;
      cout << tokenList->at( i ).value << " " ;
    } // for
    
    cout << endl ;
  } // PrintVector()

} ; // Printer

  // Initialize
  int Printer::sCurSubroutine_ = 0 ;
  int Printer::sPrintLeftBracketCount_ = 0;

// ---------------------------Exception---------------------------------- //

class Exception {

public:
  ErrorType mErrorType_ ;
  string mCurrent_func_name_ ;
  string mCurrentToken_ ;
  Node* mCurrentNode_ ;

  Exception( ErrorType errorType ) {
    mErrorType_ = errorType ;
    mCurrent_func_name_ = "\0" ;
    mCurrentToken_ = "\0" ;
    mCurrentNode_ = NULL ;
  } // Exception()

  Exception( ErrorType errorType, string& token ) {
    mErrorType_ = errorType ;
    mCurrent_func_name_ = "\0" ;
    mCurrentToken_ = token ;
    mCurrentNode_ = NULL ;
  } // Exception()

  Exception( ErrorType errorType, string& func_name, string& token ) {
    mErrorType_ = errorType ;
    mCurrent_func_name_ = func_name ;
    mCurrentToken_ = token ;
    mCurrentNode_ = NULL ;
  } // Exception()

  Exception( ErrorType errorType, Node* root ) {
    mErrorType_ = errorType ;
    mCurrent_func_name_ = "\0" ;
    mCurrentToken_ = "\0" ;
    mCurrentNode_ = root ;
  } // Exception()

} ; // Exception


// ---------------------------ErrorHadling---------------------------------- //

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

  static void UNEXPECTED_RIGHTBRACKET_TOKEN_ERROR( string token ) {
    cout << "ERROR (unexpected token) :" 
         << " ')' expected when token at Line " << gLineNum
         << " Column " << gColumn
         << " is >>" << token << "<<\n" ;
  } // UNEXPECTED_RIGHTBRACKET_TOKEN_ERROR()

  static void UNDEFINEDID_ERROR( string token ) {
    cout << "Undefined identifier :'" << token << "\n" ;
  } // UNDEFINEDID_ERROR()

  static void DIVID_ZERO_ERROR( string token ) {
    cout << "ERROR (division by zero) : " << token << "\n" ;
  } // DIVID_ZERO_ERROR()

  static void MEMORY_NOT_ENOUGH_ERROR() {
    cout << "Memory dosen't enough\n" ;
  } // DIVID_ZERO_ERROR()

  static void UNBOUND_SYMBOL_ERROR( string token ) {
    cout << "ERROR (unbound symbol) : " << token << "\n" ;
  } // UNBOUND_SYMBOL_ERROR()

  static void NON_LIST_ERROR( Node* root ) {
    cout << "ERROR (non-list) : " ;
    Printer::PrettyPrint( root ) ;
    
  } // NON_LIST_ERROR()

  static void NON_FUNCTION_ERROR( string token ) {
    cout << "ERROR (attempt to apply non-function) : " << token << "\n" ;
  } // NON_FUNCTION_ERROR()
  
  static void LEVEL_OF_EXIT_ERROR() {
    cout << "ERROR (level of EXIT)\n" ;
  } // LEVEL_OF_EXIT_ERROR()

  static void LEVEL_OF_DEFINE_ERROR() {
    cout << "ERROR (level of DEFINE)\n" ;
  } // LEVEL_OF_DEFINE_ERROR()

  static void LEVEL_OF_CLEAN_ENVIRONMENT_ERROR() {
    cout << "ERROR (level of CLEAN-ENVIRONMENT)\n" ;
  } // LEVEL_OF_CLEAN_ENVIRONMENT_ERROR()

  static void INCORRECT_ARGUMENTS_ERROR( string token ) {
    cout << "ERROR (incorrect number of arguments) : " << token << "\n" ;
  } // INCORRECT_ARGUMENTS_ERROR()

  static void INCORRECT_DEFINE_FORMAT_ERROR( Node* root ) {
    cout << "ERROR (DEFINE format) : " ;
    Printer::PrettyPrint( root ) ;
  } // INCORRECT_DEFINE_FORMAT_ERROR()

  static void INCORRECT_ARGUMENT_TYPE_ERROR( string func_name, string token ) {
    cout << "ERROR (" << func_name << " with incorrect argument type) : " << token << "\n" ;
  } // INCORRECT_ARGUMENT_TYPE_ERROR()

  static void TESTING_ERROR( string token ) {
    cout << "Testing bug with token : " << token << "\n" ;
  } // TESTING_ERROR()


public:

  static void ErrorMessage( ErrorType type, string func_name, string token, Node* root ) {
    if ( type == NO_MORE_INPUT ) NO_MORE_INPUT_ERROR() ;
    else if ( type == NO_CLOSING_QUOTE ) NO_CLOSING_QOUTE_ERROR() ;
    else if ( type == UNRECOGNIZED_TOKEN ) UNRECOGNIZED_TOKEN_ERROR( token ) ;
    else if ( type == UNEXPECTED_TOKEN ) UNEXPECTED_TOKEN_ERROR( token ) ;
    else if ( type == UNEXPECTED_RIGHTBRACKET_TOKEN ) UNEXPECTED_RIGHTBRACKET_TOKEN_ERROR( token ) ;
    else if ( type == UNDEFINEDID ) UNDEFINEDID_ERROR( token ) ;
    else if ( type == DIVID_ZERO ) DIVID_ZERO_ERROR( token ) ;
    else if ( type == MEMORY_NOT_ENOUGH ) MEMORY_NOT_ENOUGH_ERROR() ;
    else if ( type == UNBOUND_SYMBOL ) UNBOUND_SYMBOL_ERROR( token ) ;
    else if ( type == NON_LIST ) NON_LIST_ERROR( root ) ;
    else if ( type == NON_FUNCTION ) NON_FUNCTION_ERROR( token ) ;
    else if ( type == LEVEL_OF_EXIT ) LEVEL_OF_EXIT_ERROR() ;
    else if ( type == LEVEL_OF_DEFINE ) LEVEL_OF_DEFINE_ERROR() ;
    else if ( type == LEVEL_OF_CLEAN_ENVIRONMENT ) LEVEL_OF_CLEAN_ENVIRONMENT_ERROR() ;
    else if ( type == INCORRECT_ARGUMENTS ) INCORRECT_ARGUMENTS_ERROR( token ) ;
    else if ( type == INCORRECT_DEFINE_FORMAT ) INCORRECT_DEFINE_FORMAT_ERROR( root ) ;
    else if ( type == INCORRECT_ARGUMENT_TYPE ) INCORRECT_ARGUMENT_TYPE_ERROR( func_name, token ) ;
    else if ( type == TESTING ) TESTING_ERROR( token ) ;

  } // ErrorMessage()

} ; // ErrorHadling


class TokenChecker {
  
  public:
  // Check whether Token is atom 
  static bool IsAtom( Token token ) {
    TokenType type = token.type ;
    return ( type == SYMBOL ||
             type == INT    ||
             type == FLOAT  ||
             type == STRING ||
             type == NIL    ||
             type == T      ) ;
  } // IsAtom()

  static bool IsAtom( TokenType type ) {
    return ( type == SYMBOL ||
             type == INT    ||
             type == FLOAT  ||
             type == STRING ||
             type == NIL    ||
             type == T      ) ;
  } // IsAtom()

  static bool IsReserveWord( string name ) {
    return ( name == "cons"     || name == "list"    ||
             name == "quote"    || name == "define"  ||
             name == "car"      || name == "cdr"     ||
             name == "atom?"    || name == "pair?"   ||
             name == "list?"    || name == "null?"   ||
             name == "integer?" || name == "real?"   ||
             name == "number?"  || name == "string?" ||
             name == "boolean?" || name == "symbol?" ||
             name == "+"    || name == "-"   ||
             name == "*"    || name == "/"   ||
             name == "and"  || name == "or"  ||
             name == ">"    || name == ">="  ||
             name == "<"    || name == "<="  ||
             name == "="    || name == "not" ||
             name == "string>?" || name == "string<?"      ||
             name == "string=?" || name == "string-append" ||
             name == "eqv?"     || name == "equal?"        ||
             name == "begin"    || 
             name == "if"       || name == "cond" ||
             name == "clean-environment"          ||
             name == "exit" ) ;
  } // IsReserveWord()

} ; // TokenCheck

/* ---------------------------DataBase------------------------------------ */

class Hash {

  public:
  static int Eval_Key( string str ) {
    int hashkey = 0 ;

    for ( int i = 0 ; i < str.size() ; i++ ) {
      hashkey = ( hashkey + str[i] ) % MAX_MEMORY_SIZE ;
    } // for

    return hashkey ;
  } // Eval_Key

} ; // Hash

// ---------------------------Symbol Table------------------------------------ //
/**
 * Symbol Table using hash function to store name and value.
 * 
*/
class SymbolTable {
  
  private:
  vector<Symbol*> *mSymbolTable_ ;

  Symbol* CreateASymbol( string name, Node* head ) {
    Symbol* newSymbol = new Symbol() ;
    newSymbol->name = name ;
    newSymbol->binding = head ;
    newSymbol->next = NULL ;
    return newSymbol ;
  } // CreateASymbol()

  // Evaluateing key and dealing with collision
  // case1: if there is same name symbol, return it's key
  // case2: if there is no same name symbol, return the key 
  // which is empty ( NULL or name == "\0" ) and nearest to original hash key
  int GetKey( string name ) {
    int key = Hash::Eval_Key( name ) ;
    int empty_node_Key = -1 ;
    int count = 0 ;
    Symbol* cur = mSymbolTable_->at( key ) ;
    
    // if same name, return the key
    if ( cur != NULL && cur->name == name ) return key ;

    // if the not NULL means the key had used before
    while ( cur != NULL ) {
      if ( cur->name == name ) return key ;
      else if ( cur->name == "\0" && empty_node_Key == -1 ) empty_node_Key = key ;

      key++ ;
      key %= MAX_MEMORY_SIZE ; // avoid out of size ;
      cur = mSymbolTable_->at( key ) ;

      // if all memory are fulled
      if ( count >= MAX_MEMORY_SIZE ) throw Exception( MEMORY_NOT_ENOUGH ) ;
    } // while

    // if there is any name = "\0"
    if ( empty_node_Key != -1 ) return empty_node_Key ;

    return key ;

  } // GetKey()
  
  // Free the memory space of a tree 
  void FreeUpValue( Node* head ) {

    if ( head == NULL ) return ;

    FreeUpValue( head->left ) ;
    FreeUpValue( head->right ) ;

    delete head ;

    head = NULL ;

  } // FreeUpValue()

  public:
  SymbolTable() {
    mSymbolTable_ = new vector<Symbol*>( MAX_MEMORY_SIZE ) ;
  } // SymbolTable()

  Node* Get( string name ) {
    int key = GetKey( name ) ;

    if ( mSymbolTable_->at( key ) == NULL ) throw Exception( UNBOUND_SYMBOL, name ) ;

    // Get the last symbol
    Symbol* cur = mSymbolTable_->at( key ) ;
    while ( cur->next != NULL ) cur = cur->next ;

    return cur->binding ;
  } // Get()

  // if symbol exit, return true
  bool Find( string name ) {
    int key = GetKey( name ) ;
    return ( mSymbolTable_->at( key ) == NULL ) ;
  } // Find()

  void Insert( string name, Node* head ) {
    int key = GetKey( name ) ;
    Symbol* cur = mSymbolTable_->at( key ) ;

    // cout << name << ":" << key << endl ; // debug

    // check whether collision occured
    // Note: In fact it's checked in GetKey Function
    if ( cur == NULL ) mSymbolTable_->at( key ) = CreateASymbol( name, head ) ;

    // if the node dosen't free up, free it and appendit
    else if ( cur->name == "\0" ) {
      delete mSymbolTable_->at( key ) ;
      mSymbolTable_->at( key ) = CreateASymbol( name, head ) ;
    } // else if

    // if collision and the symbol has same name
    else if ( cur->name == name ) {
      // note: notice define repeatly problem
      // go to the last not NULL symbol
      while ( cur->next != NULL ) cur = cur->next ;
      // append the new same name symbol to last
      cur->next = CreateASymbol( name, head ) ;
    } // else if

    // if collision 
    else {
      throw Exception( MEMORY_NOT_ENOUGH ) ;
    } // else

    // cout << "define successful!\n" ;
    
  } // Insert()

  // Only Delete the content but not to free up space, so the pointer would be exist
  void Delete( string name ) {

    int key = GetKey( name ) ;

    if ( mSymbolTable_->at( key ) == NULL ) return ;

    mSymbolTable_->at( key )->name = "\0" ;
    // free the tree memory
    FreeUpValue( mSymbolTable_->at( key )->binding ) ;
    mSymbolTable_->at( key )->binding = NULL ;

    // TODO next Case
  } // Delete()
  
  // clear symbol table
  void Clear() {
    mSymbolTable_->clear() ;
    mSymbolTable_ = new vector<Symbol*>( MAX_MEMORY_SIZE ) ;
  } // Clear()

} ;

/* ---------------------------Processing Stage--------------------------- */
// ---------------------------Reader------------------------------------- //

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
    char peek = cin.peek() ;
    return ( peek == EOF ) ? EOF : peek ;
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
    char peek = mReader_.PeekChar() ;
    while ( peek != '\n' && peek != EOF ) {
      mReader_.GetChar() ;
      peek = mReader_.PeekChar() ;
    } // while
  } // Remove_RestOfCharInThisLine()

  char PreProcessingPeek() {
    char peekChar = mReader_.PeekChar() ;
    
    while ( IsWhiteSpace( peekChar ) ||
            IsComment( peekChar )    ||
            IsNewLine( peekChar ) ) {

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

  // To check whether the token is Dot
  bool IsDot( char ch ) {
    return ( ch == '.' ) ;
  } // IsDot()

  // To check whether the token is sign
  bool IsSign( char ch ) {
    return ( ch == '+' || ch == '-' ) ;
  } // IsSign()

  // To check whether the char is decimal
  bool IsDigit( char ch ) {
    return SystemFunctions::IsDigit( ch ) ;
  } // IsDigit()

  // To check whether the token is EOF
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
    oneToken.primitiveType = NONE ;

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
      if ( !IsSExp() ) throw Exception( UNEXPECTED_TOKEN, mCurToken.value ) ;
      GetNextToken() ;

      // { <S-exp> }
      while ( IsSExp() ) {
        GetNextToken() ;
      } // while

      // [ DOT <S-exp> ]
      if ( IsDot() ) {
        GetNextToken() ;
        if ( !IsSExp() ) throw Exception( UNEXPECTED_TOKEN, mCurToken.value ) ;
        GetNextToken() ;
      } // if

      // RIGHT-PAREN
      if ( !IsRightParen() ) throw Exception( UNEXPECTED_RIGHTBRACKET_TOKEN, mCurToken.value ) ;

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

private:

  bool IsBasicArithmeticOperator( string op ) {
    return ( op == "+"   || op == "-"  ||
             op == "*"   || op == "/"  ||
             op == "and" || op == "or" ||
             op == ">"   || op == ">=" ||
             op == "<"   || op == "<=" ||
             op == "="   || op == "not" ) ;
  } // IsBasicArithmeticOperator

  bool IsBasicStringOperator( string op ) {
    return ( op == "string-append" ||
             op == "string>?"      ||
             op == "string<?"      ||
             op == "string=?" ) ;
  } // IsBasicArithmeticOperator

public:

  Node* ArithmeticCheck( Node* root ) {

    Node* cur = root ;
    string arithmetic_Operator = cur->left->content->value ;

    int operandCount = 0 ;
    bool hasInt = false ;
    bool hasFloat = false ;
    bool hasString = false ;

    // if there is no argument
    if ( cur->right == NULL ) throw Exception( INCORRECT_ARGUMENTS, arithmetic_Operator ) ;

    // Through all nodes: check bypass argument
    while ( cur->right != NULL && cur->right->content == NULL ) {
      // Go to next parameter
      cur = cur->right ;
      operandCount++ ;

      // Get the operand
      Node* opnd = cur->left ;
      if ( opnd == NULL ) throw Exception( TESTING, opnd->content->value ) ;
      
      // Recording arguments type and check Divide Zero ERROR
      if ( opnd->content != NULL ) {
        if ( opnd->content->type == INT ) hasInt = true ;
        else if ( opnd->content->type == FLOAT ) hasFloat = true ;
        else if ( opnd->content->type == STRING ) hasString = true ;

        // if divid zero
        if ( arithmetic_Operator == "/" &&
             operandCount > 1           &&
             opnd->content->value == "0" ) throw Exception( DIVID_ZERO, arithmetic_Operator ) ; 

      } // if

      // Check if operand type correct?
      if ( IsBasicArithmeticOperator( arithmetic_Operator ) && hasString )
        throw Exception( INCORRECT_ARGUMENT_TYPE, arithmetic_Operator, opnd->content->value ) ;
      else if ( IsBasicStringOperator( arithmetic_Operator ) ) {
        if ( hasInt || hasFloat ) 
          throw Exception( INCORRECT_ARGUMENT_TYPE, arithmetic_Operator, opnd->content->value ) ;
      } // else if

    } // while 
    
    if ( cur->right != NULL && cur->right->content != NULL ) throw Exception( NON_LIST, root ) ;

    // check if not has only one parameter
    if ( arithmetic_Operator == "not" && operandCount > 1 ) 
      throw Exception( INCORRECT_ARGUMENTS, arithmetic_Operator ) ;
    else if ( arithmetic_Operator != "not" && operandCount < 2 ) throw Exception( INCORRECT_ARGUMENTS, arithmetic_Operator ) ;
    

    return root ;

  } // ArithmeticCheck()

} ; // SemanticsAnalyzer


// ---------------------------Tree Data Structure------------------------ //

class Tree {

private:
  
  vector<Node*> *mRoots_ ; // store all of tree roots
  vector<Token> *mTokensQueue_ ;

  Token mCurToken_ ;
  int mCurSubroutine_ ;
  
  // create and initialize new Atom
  Node* Create_A_New_Node() {
    Node *oneNode = new Node() ;

    oneNode->left = NULL ;
    oneNode->right = NULL ;
    oneNode->content = NULL ;
    oneNode->subroutineNum = mCurSubroutine_ ;
    oneNode->isConsBegin = false ;

    return oneNode ;
  } // Create_A_New_Node()

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
    mTokensQueue_ = PreProcessingTokenList( tokenList ) ;
    
  } // SetTokenList()

  // Dealing with format: '(...) -> (quote(...))
  vector<Token>* PreProcessingTokenList( vector<Token>* tokenList  ) {
    vector<Token> *queue = new vector<Token>() ;
    // record how many index of bracket in quote
    vector<int> *bracketIndex = new vector<int>() ;

    while ( ! tokenList->empty() ) {
      Token curToken = GetNextToken( tokenList ) ;

      if ( curToken.type == QUOTE  ) {
        // ('.(nil))
        queue->push_back( SystemFunctions::CreateNewToken( "(", LEFT_PAREN ) ) ;
        queue->push_back( curToken ) ; // quote
        bracketIndex->push_back( 0 ) ;
      } // if
      else {
        queue->push_back( curToken ) ;

        // if it has quote 
        if ( ! bracketIndex->empty() ) {
          // to record
          if ( curToken.type == LEFT_PAREN ) bracketIndex->back()++ ;
          else if ( curToken.type == RIGHT_PAREN ) bracketIndex->back()-- ;

          // if there is no index of bracket, fill in right bracket
          while ( ! bracketIndex->empty() && bracketIndex->back() == 0 ) {
            bracketIndex->pop_back() ;
            queue->push_back( SystemFunctions::CreateNewToken( ")", RIGHT_PAREN ) ) ;
          } // while
        } // if 

      } // else
    } // while

    return queue ;
  
  } // PreProcessingTokenList()

  Token GetNextToken( vector<Token>* tokenList ) {
    Token curToken = tokenList->front() ; // Get first token
    tokenList->erase( tokenList->begin() ) ;
    return curToken ;
  } // GetNextToken()

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
    newToken->type = mCurToken_.type ;
    newToken->value = mCurToken_.value ;
    newToken->primitiveType = mCurToken_.primitiveType ;
    return newToken ;
  } // CopyCurToken()

  Node* SaveToken_with_NewNode() {
    Node* temp = Create_A_New_Node() ;
    temp->content = CopyCurToken() ;
    // cout << "Save Token: " << " (" << temp->subroutineNum << ") " 
    //      << temp->content->value << endl ; // DEBUG 
    return temp ;
  } // SaveToken_with_NewNode()

  // Build the constructure
  Node* BuildCons() {
    
    mCurSubroutine_++ ;
    Node* root = Create_A_New_Node() ;
    Node* cur = root ;

    // buildind a sub tree
    // ---------- Left Bracket ( ---------- //
    if ( mCurToken_.type == LEFT_PAREN ) {
      root->isConsBegin = true ;

      GetNextToken() ;

      // case1: ( A B C ) save left atom to node
      // case2: ( A ( B C ) D ) if meet the left bracket, create new node to connect it
      while ( TokenChecker::IsAtom( mCurToken_ )          ||
              mCurToken_.type == LEFT_PAREN ||
              mCurToken_.type == QUOTE ) {

        // if is atom save to left node
        if ( TokenChecker::IsAtom( mCurToken_ ) ) cur->left = SaveToken_with_NewNode() ;
        // if is subtree, save to left node
        else if ( mCurToken_.type == LEFT_PAREN ) cur->left = BuildCons() ;
        // if is qoute, save to left node
        else if ( mCurToken_.type == QUOTE ) {

          // if is '(') keep the left bracket exist

          cur->left = SaveToken_with_NewNode() ;
          cur->right = Create_A_New_Node() ;
          cur = cur->right ;
          GetNextToken() ;
          cur->left = BuildCons() ;
        } // else if
          
        // get the next token
        GetNextToken() ;

        // if the next token got is a dot or ), cur can't move to and create right node
        // cause the right node need to check 
        if ( mCurToken_.type != DOT && mCurToken_.type != RIGHT_PAREN ) {  
          cur->right = Create_A_New_Node() ;
          cur = cur->right ;
        } // if
        
      } // while

      // case3: ( A . B )
      if ( mCurToken_.type == DOT ) {
        GetNextToken() ; // get next token

        if ( TokenChecker::IsAtom( mCurToken_ ) ) cur->right = SaveToken_with_NewNode() ;
        else if ( mCurToken_.type == LEFT_PAREN ) cur->right = BuildCons() ;
        GetNextToken() ; // get next token

      } // if: Dot

      if ( mCurToken_.type == RIGHT_PAREN ) {
        mCurSubroutine_-- ;
        return root ;
      } // if

    } // if
    // ---------- Atom ---------- //
    else if ( TokenChecker::IsAtom( mCurToken_ ) ) {
      // ex. 'a ''()
      cur->content = CopyCurToken() ;
      return root ;
    } // else if

    return root ;

  } // BuildCons()

public:
  
  Tree() {
    mRoots_ = new vector<Node*>() ;
    mTokensQueue_ = new vector<Token>() ;
    mCurSubroutine_ = 0 ;
  } // Tree()

  // Create A New Tree For Current Token List
  void CreateTree( vector<Token>* tokenList ) {

    // ---------- Initialize ---------- //
    // push tokenList in and preprocess
    SetTokenList( tokenList ) ;
    mCurSubroutine_ = 0 ;

    // null node in root
    Node* treeRoot = Create_A_New_Node() ;
    GetNextToken() ;

    // ---------- Building A Tree ---------- //
    treeRoot = BuildCons() ;

    // ---------- Finish ---------- //
    mRoots_->push_back( treeRoot ) ;
  } // CreateTree()

  // return current tree root
  Node *GetCurrentRoot() {
    if ( !mRoots_->empty() ) return mRoots_->back() ;
    return NULL ;
  } // GetCurrentRoot()

} ; // Tree


// ---------------------------Evaluator---------------------------------- //

class Evaluator {

private:
  
  SymbolTable mSymbolTable_ ;
  SemanticsAnalyzer mSemanticsAnalyzer_ ;
  vector<Token> *mCommand_ ;
  int mLevel_ ;
  bool mIsQuit ;

  // Check the tree arguments is equal to ?
  bool IsArgCountEqualTo( Node* root, int arg_count ) {
    Node* cur = root ;

    // if arg < arg_count
    for ( int i = 0 ; i < arg_count ; i++ ) {
      if ( cur == NULL || cur->left == NULL ) return false ;
      cur = cur->right ;
    } // for

    // if arg > arg_count
    // empty right node
    if ( cur == NULL ) return true ;
    // right node is NIL and only one
    else if ( cur != NULL          &&
              cur->content != NULL &&
              cur->left == NULL    &&
              cur->right == NULL   && 
              cur->content->type == NIL ) return true ;
    
    return false ;
  } // CheckArgCount()

  Node* CreateNode( string name, TokenType type ) {
    Token* token = new Token() ;
    Node* node = new Node() ;
    node->content = token ;
    node->content->value = name ;
    node->content->type = type ;
    node->content->primitiveType = NONE ;
    node->left = NULL ;
    node->right = NULL ;
    node->isConsBegin = false ;
    node->subroutineNum = 1 ;

    return node ;
  } // CreateNode() ;

  // return left node
  Node* GetArgument( Node* cur ) {
    return cur->left ;
  } // GetArgument()

  bool IsPared( Node* root ) {
    Node* cur = root ;
    
    if ( cur->left == NULL ) return false ;

    while ( cur->right != NULL && cur->right->content == NULL ) {
      cur = cur->right ;
      if ( cur->left == NULL ) return false ;
    } // while()

    return true ;
  } // IsPared()

  bool IsList( Node* root ) {
    Node* cur = root ;
    
    if ( cur->left == NULL ) return false ;

    while ( cur->right != NULL ) {
      cur = cur->right ;
      if ( cur->content != NULL ) return false ;
      else if ( cur->left == NULL ) return false ;
    } // while()

    return true ;
  } // IsList()

  bool IsNull( Node* root ) {
    TokenType type = root->content->type ;
    return ( type == NIL ) ;
  } // IsNull()

  bool IsInt( Node* root ) {
    TokenType type = root->content->type ;
    return ( type == INT ) ;
  } // IsInt()

  bool IsReal( Node* root ) {
    TokenType type = root->content->type ;
    return ( type == INT || type == FLOAT ) ;
  } // IsReal()

  bool IsString( Node* root ) {
    TokenType type = root->content->type ;
    return ( type == STRING ) ;
  } // IsString()

  bool IsBoolean( Node* root ) {
    TokenType type = root->content->type ;
    return ( type == T || type == NIL ) ;
  } // IsBoolean()

  bool IsSymbol( Node* root ) {
    TokenType type = root->content->type ;
    return ( type == SYMBOL ) ;
  } // IsSymbol()

  Node* BasicArithmaticCalculate( string opnd, Node* root ) {

    // root only have parameters, not including function name
    Node* cur = root ;

    // Get the first number
    float result = SystemFunctions::To_float( cur->left->content->value ) ;
    TokenType type = INT ;

    // Continue to get next number
    while ( cur->right != NULL ) {
      cur = cur->right ;

      // Check Type
      if ( cur->left->content->type == FLOAT ) type = FLOAT ;

      // Calculating
      if ( opnd == "+" ) result += SystemFunctions::To_float( cur->left->content->value ) ;
      else if ( opnd == "-" ) result -= SystemFunctions::To_float( cur->left->content->value ) ;
      else if ( opnd == "*" ) result *= SystemFunctions::To_float( cur->left->content->value ) ;
      else if ( opnd == "/" ) result /= SystemFunctions::To_float( cur->left->content->value ) ;
    } // while

    return CreateNode( SystemFunctions::To_string( result ), type ) ;
  } // BasicArithmaticCalculate()


  // ---------- Functions ---------- //
  // cons    (2), list   (>=0)
  Node* Eval_Constructor( Node* root ) {

    Node* cur = root ;
    string function_name = cur->left->content->value ;

    // Go next node: first bypass argument
    cur = cur->right ;

    // ---------- Function: cons ---------- //
    if ( function_name == "cons" ) {

      // check arguments count
      if ( ! IsArgCountEqualTo( cur, 2 ) ) throw Exception( INCORRECT_ARGUMENTS, function_name ) ;

      Node* first_arg = Evaluate( GetArgument( cur ) ) ;
      Node* scecond_arg = Evaluate( GetArgument( cur->right ) ) ;

      // Paired Node
      root->left = first_arg ;
      root->right = scecond_arg ;

      return root ;
      
    } // if

    // ---------- Function: list ---------- //
    else if ( function_name == "list" ) {
      
      // if args == 0
      if ( cur == NULL ) throw Exception( INCORRECT_ARGUMENTS, function_name ) ;
      
      Node* result = cur ;
      
      cur->left = Evaluate( GetArgument(cur) ) ;
      while( cur->right != NULL ) {
        cur = cur->right ;
        cur->left = Evaluate( GetArgument(cur) ) ;
      } // while

      return result ;

    } // else if

    return cur ;
  } // Eval_Constructor()
  
  // '       (1), quote  (1)
  Node* Eval_Quote_Bypass( Node* root ) {

    Node* cur = root ;
    string function_name = cur->left->content->value ;
    // if ( function_name != "quote" ) throw Exception(  )

    // Go next node: first bypass argument
    cur = cur->right ;

    if ( function_name == "quote" || function_name == "\'" ) {
      if ( ! IsArgCountEqualTo( cur, 1 ) ) throw Exception( INCORRECT_ARGUMENTS, function_name ) ;
      Node* first_arg = GetArgument( cur ) ;

      return first_arg ;
    } // if

    return NULL ;
  } // Eval_Quote_Bypass()

  // define  (2)
  Node* Eval_Define( Node* root ) {

    Node* cur = root ;
    string function_name = cur->left->content->value ;

    // ---------- step1: check format ---------- //
    if ( function_name != "define" ) throw Exception( TESTING, function_name ) ;
    // Go next node: first bypass argument
    cur = cur->right ;

    // Paired tree constructure: ( define a . 5 ) case
    if ( cur->right != NULL && cur->right->content != NULL ) throw Exception( NON_LIST, root ) ;
    // check arguments count and is no NULL
    if ( ! IsArgCountEqualTo( cur, 2 ) ) throw Exception( INCORRECT_DEFINE_FORMAT, root ) ;

    Node* first_arg = GetArgument( cur ) ;
    // check if define name has wrong format or naming 
    if ( first_arg->content == NULL ) throw Exception( INCORRECT_DEFINE_FORMAT, root ) ;
    else if ( first_arg->content->type != SYMBOL ) throw Exception( INCORRECT_DEFINE_FORMAT, root ) ;
    else if ( TokenChecker::IsReserveWord( first_arg->content->value ) ) throw Exception( INCORRECT_DEFINE_FORMAT, root ) ;


    // ---------- step2: define ---------- //
    Node* scecond_arg = Evaluate( GetArgument( cur->right ) ) ;

    mSymbolTable_.Insert( first_arg->content->value, scecond_arg ) ;

    cout << first_arg->content->value << " defined\n" ;
    return NULL ;
  } // Eval_Define()

  // car     (1), cdr    (1)
  Node* Eval_Part_Accessors( Node* root ) {
    Node* cur = root ;
    string function_name = cur->left->content->value ;

    // Go next node: first bypass argument
    cur = cur->right ;

    // check arguments count
    if ( cur->right != NULL && cur->right->content != NULL ) throw Exception( NON_LIST, root ) ;
    else if ( ! IsArgCountEqualTo( cur, 1 ) ) throw Exception( INCORRECT_ARGUMENTS, function_name ) ;
    
    Node* result = Evaluate( GetArgument( cur ) ) ;
    if ( result->content != NULL ) 
      throw Exception( INCORRECT_ARGUMENT_TYPE, function_name, result->content->value ) ;

    // ---------- Function: car ---------- //
    if ( function_name == "car" ) {
      result = result->left ;
      if ( result == NULL ) return CreateNode( "nil", NIL ) ; // return nil : it seems need to return error
    } // if
    // ---------- Function: cdr ---------- //
    else if ( function_name == "cdr" ) {
      result = result->right ;
      if ( result == NULL ) return CreateNode( "nil", NIL ) ; // return nil
    } // else if
    else throw Exception( TESTING, function_name ) ;
    
    return result ;
  } // Eval_Part_Accessors()

  // atom? pair? list? null? integer? real? number? string? boolean? symbol? (1)
  Node* Eval_Primitive_Predicate( Node* root ) {
    Node* cur = root ;
    string function_name = cur->left->content->value ;
    // Go next node: first bypass argument
    cur = cur->right ;
    Node* first_arg = Evaluate( GetArgument( cur ) ) ;

    if ( ! IsArgCountEqualTo( cur, 1 ) ) throw Exception( INCORRECT_ARGUMENTS, function_name ) ;

    // ---------- Function: atom? ---------- //
    if ( function_name == "atom?" ) {
      if ( first_arg->content == NULL ) return CreateNode( "nil", NIL ) ;
      else if ( TokenChecker::IsAtom( first_arg->content->type ) ) return CreateNode( "#t", T ) ;
      else return CreateNode( "nil", NIL ) ;
    } // if

    // ---------- Function: pair? ---------- //
    else if ( function_name == "pair?" ) {
      if ( first_arg->content != NULL ) return CreateNode( "nil", NIL ) ;
      else if ( IsPared( first_arg ) ) return CreateNode( "#t", T ) ;
      else return CreateNode( "nil", NIL ) ;
    } // else if
    
    // ---------- Function: list? ---------- //
    else if ( function_name == "list?" ) {
      if ( first_arg->content != NULL ) return CreateNode( "nil", NIL ) ;
      else if ( IsList( first_arg ) ) return CreateNode( "#t", T ) ;
      else return CreateNode( "nil", NIL ) ;
    } // else if

    // ---------- Function: null? ---------- //
    else if ( function_name == "null?" ) {
      if ( first_arg->content == NULL ) return CreateNode( "nil", NIL ) ;
      else if ( IsNull( first_arg ) ) return CreateNode( "#t", T ) ;
      else return CreateNode( "nil", NIL ) ;
    } // else if

    // ---------- Function: integer? ---------- //
    else if ( function_name == "integer?" ) {
      if ( first_arg->content == NULL ) return CreateNode( "nil", NIL ) ;
      else if ( IsInt( first_arg ) ) return CreateNode( "#t", T ) ;
      else return CreateNode( "nil", NIL ) ;
    } // else if

    // ---------- Function: real? number?---------- //
    else if ( function_name == "real?" || function_name == "number?" ) {
      if ( first_arg->content == NULL ) return CreateNode( "nil", NIL ) ;
      else if ( IsReal( first_arg ) ) return CreateNode( "#t", T ) ;
      else return CreateNode( "nil", NIL ) ;
    } // else if

    // ---------- Function: string? ---------- //
    else if ( function_name == "string?" ) {
      if ( first_arg->content == NULL ) return CreateNode( "nil", NIL ) ;
      else if ( IsString( first_arg ) ) return CreateNode( "#t", T ) ;
      else return CreateNode( "nil", NIL ) ;
    } // else if

    // ---------- Function: boolean? ---------- //
    else if ( function_name == "boolean?" ) {
      if ( first_arg->content == NULL ) return CreateNode( "nil", NIL ) ;
      else if ( IsBoolean( first_arg ) ) return CreateNode( "#t", T ) ;
      else return CreateNode( "nil", NIL ) ;
    } // else if
    
    // ---------- Function: symbol? ---------- //
    else if ( function_name == "symbol?" ) {
      if ( first_arg->content == NULL ) return CreateNode( "nil", NIL ) ;
      else if ( IsSymbol( first_arg ) ) return CreateNode( "#t", T ) ;
      else return CreateNode( "nil", NIL ) ;
    } // else if

    return root ;
  } // Eval_Primitive_Predicate()

  // not (1) + - * / and or > >= < <= = string-append string>? string<? string=? (>=2)
  Node* Eval_Basic_Arithmetic( Node* root ) {
    
    Node* cur = root ;
    string opnd = cur->left->content->value ;

    // if there are some nested list, they will expand in this while loop
    while ( cur->right != NULL ) {
      cur = cur->right ;
      cur->left = Evaluate( GetArgument( cur ) ) ;
    } // while

    // check if the format is correct
    Node *FormatCorrectRoot = mSemanticsAnalyzer_.ArithmeticCheck( root ) ;
    Node *ByPassParameters = FormatCorrectRoot->right ;

    if ( opnd == "+" || opnd == "-" || opnd == "*" || opnd == "/" )
      return BasicArithmaticCalculate( opnd, ByPassParameters ) ;

    return FormatCorrectRoot ;
  } // Eval_Basic_Arithmetic()

  // eqv?    (2), equal? (2)
  Node* Eval_Equivalence( Node* root ) {
    cout << "equivalence processing!\n" ;
    return root ;
  } // Eval_Equivalence()

  // begin   (>=1)
  Node* Eval_Begin( Node* root ) {
    cout << "begin processing!\n" ;
    return root ;
  } // Eval_Begin()

  // if      (2 or 3), cond (>=1)
  Node* Eval_Condition( Node* root ) {
    cout << "condition processing!\n" ;
    return root ;
  } // Eval_Condition()

  // clean-environment(0)
  Node* Eval_Clean_Environment( Node* root ) {

    string function_name = root->left->content->value ;
    if ( ! IsArgCountEqualTo( root->right, 0 ) ) throw Exception( INCORRECT_ARGUMENTS, function_name ) ;
    mSymbolTable_.Clear() ;
    
    cout << "environment cleaned\n" ;
    return NULL ;
  } // Eval_Clean_Environment()

  // exit(0)
  Node* Eval_Exit( Node* root ) {
    string function_name = root->left->content->value ;
    if ( ! IsArgCountEqualTo( root->right, 0 ) ) throw Exception( INCORRECT_ARGUMENTS, function_name ) ;
    mIsQuit = true ;
    
    return NULL ;
  } // Eval_Exit()

  // ---------- Eval ---------- //
  Node* Evaluate( Node *root ) {
    
    Node *cur = root ;
    mLevel_++ ;

    if ( cur == NULL ) return NULL ;

    // ---------- CASE1 Atom || Symbol ---------- //
    // case1: Not a list
    // root node content has a value
    if ( cur->content != NULL ) {

      // if what is being evaluated is an atom but not a symbol
      if ( TokenChecker::IsAtom( cur->content->type ) && cur->content->type != SYMBOL ) {
        // return that atom
        return cur ;
      } // if

      // else if what is being evaluated is a symbol
      else if ( cur->content->type == SYMBOL ) {
        // check whether it is bound to an S-expression or an internal function
        return mSymbolTable_.Get( cur->content->value ) ; // mSymbolTable automatically check unbound
      } // else if
    } // if

    // ---------- CASE2 (...) ---------- //
    // what is being evaluated is (...)
    // root node content is NULL
    else {
      
      // Check Functional Argument
      // First argument check: if (...) is not a (pure) list
      if ( cur->left == NULL ) throw Exception( NON_LIST ) ;
      // First argument check: if is a list
      else if ( cur->left->content == NULL ) cur->left = Evaluate( cur->left ) ;

      Node *first_argument = root->left ;

      // ---------- First Argument ---------- //
      // First argument is not a symbol
      if ( TokenChecker::IsAtom( first_argument->content->type ) &&
           first_argument->content->type != SYMBOL ) 
        throw Exception( NON_FUNCTION, first_argument->content->value ) ;
      
      // else if first argument of (...) is a symbol SYM
      else if ( first_argument->content->type == SYMBOL || first_argument->content->type == QUOTE ) {

        // check if the SYM is the name of a known function
        CheckPrimitiveSymbol( first_argument->content ) ;

        // Get Symbol Binding
        if ( first_argument->content->primitiveType == NONE ) {
          first_argument = mSymbolTable_.Get( first_argument->content->value )  ;
          // check if the SYM is the name of a known function
          CheckPrimitiveSymbol( first_argument->content ) ;
          if ( first_argument->content->primitiveType == NONE )
            throw Exception( NON_FUNCTION, first_argument->content->value ) ;
        } // if

        Token* functional_token = first_argument->content ;

        // if the current level is not the top level, and SYM is 'clean-environment'
        // or 'define' or　'exit'
        if ( functional_token->primitiveType == EXIT && mLevel_ != 1 )
          throw Exception( LEVEL_OF_EXIT ) ;

        else if ( functional_token->primitiveType == DEFINE && mLevel_ != 1  )
          throw Exception( LEVEL_OF_DEFINE ) ;

        else if ( functional_token->primitiveType == CLEAN_ENVIRONMENT &&  mLevel_ != 1  )
          throw Exception( LEVEL_OF_CLEAN_ENVIRONMENT ) ;
          
          
        // ---------- Eval Function ---------- //
        // ---------- CONSTRUCTOR ---------- //
        if ( functional_token->primitiveType == CONSTRUCTOR ) {
          return Eval_Constructor( cur ) ;
        } // if

        // ---------- QUOTE_BYPASS ---------- //
        else if ( functional_token->primitiveType == QUOTE_BYPASS ) {
          return Eval_Quote_Bypass( cur ) ;
        } // else if

        // ---------- DEFINE ---------- //
        else if ( functional_token->primitiveType == DEFINE ) {
          return Eval_Define( cur ) ;

        } // else if

        // ---------- PART_ACCESSORS ---------- //
        else if ( functional_token->primitiveType == PART_ACCESSORS ) {
          return Eval_Part_Accessors( cur ) ;

        } // else if

        // ---------- PRIMITIVE_PREDICTATE ---------- //
        else if ( functional_token->primitiveType == PRIMITIVE_PREDICTATE ) {
          return Eval_Primitive_Predicate( cur ) ;

        } // else if

        // ---------- BASIC_ARITHMETIC ---------- //
        else if ( functional_token->primitiveType == BASIC_ARITHMETIC ) {
          return Eval_Basic_Arithmetic( cur ) ;

        } // else if

        // ---------- EQUIVALENCE ---------- //
        else if ( functional_token->primitiveType == EQUIVALENCE ) {
          return Eval_Equivalence( cur ) ;

        } // else if

        // ---------- BEGIN ---------- //
        else if ( functional_token->primitiveType == BEGIN ) {
          return Eval_Begin( cur ) ;
        } // else if

        // ---------- CONDITIONAL ---------- //
        else if ( functional_token->primitiveType == CONDITIONAL ) {
          return Eval_Condition( cur ) ;
        } // else if

        // ---------- CLEAN_ENVIRONMENT ---------- //
        else if ( functional_token->primitiveType == CLEAN_ENVIRONMENT ) {
          return Eval_Clean_Environment( cur ) ;
        } // else if

        // ---------- EXIT ---------- //
        else if ( functional_token->primitiveType == EXIT ) {
          return Eval_Exit( cur ) ;
        } // else if
        
        else {}
        
      } // else if
      
    } // else

    return cur ;
    /*

      else if first argument of (...) is a symbol SYM

        check whether SYM is the name of a function (i.e., check whether 「SYM has a
                                          binding, and that binding is an internal function」)

        if SYM is the name of a known function

          if the current level is not the top level, and SYM is 'clean-environment' or    
              or 'define' or　'exit'

            ERROR (level of CLEAN-ENVIRONMENT) / ERROR (level of DEFINE) / ERROR (level of EXIT)

          else if SYM is 'define' or 'set!' or 'let' or 'cond' or 'lambda'

            check the format of this expression // 注意：此時尚未check num-of-arg
            // (define symbol    // 注意：只能宣告或設定 非primitive的symbol (這是final decision!)
            //         S-expression
            // )
            // (define ( one-or-more-symbols )
            //           one-or-more-S-expressions
            // )
            // (set! symbol
            //       S-expression
            // )
            // (lambda (zero-or-more-symbols)
            //           one-or-more-S-expressions
            // )
            // (let (zero-or-more-PAIRs)
            //        one-or-more-S-expressions
            // )
            // (cond one-or-more-AT-LEAST-DOUBLETONs
            // )
            // where PAIR df= ( symbol S-expression )
            //        AT-LEAST-DOUBLETON df= a list of two or more S-expressions

            if format error (包括attempting to redefine system primitive) 
              ERROR (COND format) : <the main S-exp> 
              or
              ERROR (DEFINE format) : <the main S-exp> // 有可能是因為redefining primitive之故
              or
              ERROR (SET! format) : <the main S-exp>    // 有可能是因為redefining primitive之故
              or
              ERROR (LET format) : <the main S-exp>     // 有可能是因為redefining primitive之故
              or
              ERROR (LAMBDA format) : <the main S-exp>  // 有可能是因為redefining primitive之故

            evaluate ( ... ) 

            return the evaluated result (and exit this call to eval())

          else if SYM is 'if' or 'and' or 'or'

            check whether the number of arguments is correct

            if number of arguments is NOT correct
              ERROR (incorrect number of arguments) : if

            evaluate ( ... ) 

            return the evaluated result (and exit this call to eval())

          else // SYM is a known function name 'abc', which is neither 
                // 'define' nor 'let' nor 'cond' nor 'lambda'

            check whether the number of arguments is correct

            if number of arguments is NOT correct
              ERROR (incorrect number of arguments) : abc

        else // SYM is 'abc', which is not the name of a known function

          ERROR (unbound symbol) : abc
          or
          ERROR (attempt to apply non-function) : ☆ // ☆ is the binding of abc

      else // the first argument of ( ... ) is ( 。。。 ), i.e., it is ( ( 。。。 ) ...... )

        evaluate ( 。。。 )

        // if any error occurs during the evaluation of ( 。。。 ), we just output an
        // an appropriate error message, and we will not proceed any further

        if no error occurs during the evaluation of ( 。。。 ) 

          check whether the evaluated result (of ( 。。。 )) is an internal function

          if the evaluated result (of ( 。。。 )) is an internal function

            check whether the number of arguments is correct

            if num-of-arguments is NOT correct
              ERROR (incorrect number of arguments) : name-of-the-function
              or
              ERROR (incorrect number of arguments) : lambda expression 
                                                            // in the case of nameless functions

          else // the evaluated result (of ( 。。。 )) is not an internal function
            ERROR (attempt to apply non-function) : ☆ //  ☆ is the evaluated result

      end of 「else the first argument of ( ... ) is ( 。。。 )」
        
      eval the second argument S2 of (the main S-expression) ( ... )

      if the type of the evaluated result is not correct 
        ERROR (xxx with incorrect argument type) : the-evaluated-result
        // xxx must be the name of some primitive function!

      if no error
        eval the third argument S3 of (the main S-expression) ( ... )

      if the type of the evaluated result is not correct 
        ERROR (xxx with incorrect argument type) : the-evaluated-result

      ...

      if no error

        apply the binding of the first argument (an internal function) to S2-eval-result, 
        S3-eval-result, ... 

        if no error
          if there is an evaluated result to be returned
            return the evaluated result
          else
            ERROR (no return result) : name-of-this-function
            or
            ERROR (no return result) : lambda expression // if there is such a case ...

    end // else what is being evaluated is (...) ; we call it the main S-expression

    */
    
  } // Evaluate()

  void CheckPrimitiveSymbol( Token* token ) {
    string name = token->value ;

    // <CONSTRUCTOR> := cons list 
    if ( name == "cons" || name == "list" ) token->primitiveType = CONSTRUCTOR ;

    // <QUOTE_BYPASS> := ' quote
    else if ( name == "\'" || name == "quote" ) token->primitiveType = QUOTE_BYPASS ;

    // <DEFINE> := define
    else if ( name == "define" ) token->primitiveType = DEFINE ;

    // <PART_ACCESSORS> := car cdr
    else if ( name == "car" || name == "cdr" ) token->primitiveType = PART_ACCESSORS ;

    // <PRIMITIVE_PREDICTATE> :=
    // atom? pair? list? null? integer? real? number? string? boolean? symbol?
    else if ( name == "atom?"    || name == "pair?"   ||
              name == "list?"    || name == "null?"   ||
              name == "integer?" || name == "real?"   ||
              name == "number?"  || name == "string?" ||
              name == "boolean?" || name == "symbol?" ) token->primitiveType = PRIMITIVE_PREDICTATE ;
    
    // <BASIC_ARITHMETIC> :=
    // + - * / and or > >= < <= = not string>? string<? string=? string-append
    else if ( name == "+"    || name == "-"   ||
              name == "*"    || name == "/"   ||
              name == "and"  || name == "or"  ||
              name == ">"    || name == ">="  ||
              name == "<"    || name == "<="  ||
              name == "="    || name == "not" ||
              name == "string>?" || name == "string<?" ||
              name == "string=?" || name == "string-append" ) token->primitiveType = BASIC_ARITHMETIC ;

    // <EQUIVALENCE> := eqv? equal?
    else if ( name == "eqv?" || name == "equal?" ) token->primitiveType = EQUIVALENCE ;

    // <BEGIN> := begin
    else if ( name == "begin" ) token->primitiveType = BEGIN ;

    // <CONDITIONAL> := if cond
    else if ( name == "if" || name == "cond"  ) token->primitiveType = CONDITIONAL ;

    // <CLEAN_ENVIRONMENT> := clean-environment
    else if ( name == "clean-environment" ) token->primitiveType = CLEAN_ENVIRONMENT ;

    // <EXIT> := exit
    else if ( name == "exit" ) token->primitiveType = EXIT ;


  } // IsPremitiveSymbol()

  Token CopyToken( Node* node ) {
    Token newToken ;
    newToken.value = node->content->value ;
    newToken.type = node->content->type ;
    return newToken ;
  } // CopyToken()

public:

  Evaluator() {
    mCommand_ = new vector<Token>() ;
    mLevel_ = 0 ;
    mIsQuit = false ;
  } // Evaluator()

  // Go through Right
  Node* Execute( Node *root ) {

    // set Top Level
    mLevel_ = 0 ;

    // Evaluating
    return Evaluate( root ) ;

  } // Execute()

  bool IsExit( const Node *temp ) {

    return mIsQuit ;

    // empty node
    if ( temp == NULL ) return false ;
    // empty leftnode
    else if ( temp->left == NULL ) return false ;
    // leftnode has no value
    else if ( temp->left->content == NULL ) return false ;
    // leftnode value is not exit
    else if ( temp->left->content->value != "exit" ) return false ;
    
    // empty right node
    if ( temp->right == NULL ) return true ;
    // right node is NIL and only one
    else if ( temp->right != NULL          &&
              temp->right->content != NULL &&
              temp->right->left == NULL    &&
              temp->right->right == NULL   && 
              temp->right->content->type == NIL ) return true ;

    return false ;

  } // IsExit()

} ; // Evaluator


// ---------------------------OurScheme System--------------------------- //

// OurScheme main system
class OurScheme {

private:

  Parser mSyntaxAnalyzer_ ;
  SemanticsAnalyzer mSemanticsAnalyzer_ ;
  Tree mAtomTree_ ;
  Evaluator mEvaluator_ ;

  vector<vector<Token>*> *mTokenTable_ ; // storing all of primitive tokens
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
      // storing current tokenList to tokenTable
      mTokenTable_->push_back( tokenList ) ; // TOFIX: Current tokeList overriding

      // Building A Tree
      mAtomTree_.CreateTree( tokenList ) ;
      
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
      
      // lexical and syntax error handling
      try {
        ReadSExp() ;
        Node *curAtomRoot = mAtomTree_.GetCurrentRoot() ;

        // evaluate and sematic error handling
        try {

          // Execute Evaluate
          Node *result = mEvaluator_.Execute( curAtomRoot ) ;

          // Check if is exit
          if ( mEvaluator_.IsExit( result ) ) mQuit_ = true ;
          else Printer::PrettyPrint( result ) ; // Pretty print

        } // try
        catch ( const Exception& e ) {

          ErrorHadling::ErrorMessage( e.mErrorType_, e.mCurrent_func_name_, e.mCurrentToken_, e.mCurrentNode_ ) ;
          gLineNum = 1 ;
          gColumn = 0 ; // while read the first char, col will add 1 automatically
        } // catch

      } // try
      catch ( const Exception& e ) {
        
        ErrorHadling::ErrorMessage( e.mErrorType_, e.mCurrent_func_name_, e.mCurrentToken_, e.mCurrentNode_ ) ;
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
