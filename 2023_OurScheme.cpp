# include <iostream>
# include <stdio.h>
# include <stdlib.h>
# include <string>
# include <vector>
# include <sstream>
# include <iomanip>
# include <math.h>
# include <map>

using namespace std;

# define MAX_MEMORY_SIZE 1024

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
  INCORRECT_COND_FORMAT,
  INCORRECT_LIST_FORMAT,
  INCORRECT_LET_FORMAT,
  INCORRECT_LAMBDA_FORMAT,
  NO_RETURN_VALUE,
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
  LET,
  LAMBDA,
  SET,
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
  int address ;
  int subroutineNum ;
  bool isConsBegin ;
} ;

// Type of Data in Symbol table 
struct Symbol {
  string name ;
  Node* binding ; // record the tree root 
  Node* parameter ;
  Node* expression ;
  bool isFunction ;
  Symbol* next ;
} ;


int g_uTestNum ;
int gLineNum ;
int gColumn ;
int gAddress ;

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
    float result = 0 ;
    float decimal = 1 ;
    int sign = 1 ;
    int i = 0 ;

    // Sign Bit
    if ( str[i] == '-' ) {
      sign = -1 ;
      i++ ;
    } // if
    else if ( str[i] == '+' ) {
      i++ ;
    } // else if

    // Decimal Part
    while ( str[i] != '.' && i < str.length() ) {
      int digit = str[i] - '0' ;
      result = result * 10 + digit ;
      i++ ;
    } // while

    // Point Part
    if ( str[i] == '.' ) {
      i++ ;
      while ( i < str.length() ) {
        int digit = str[i] - '0' ;
        decimal *= 0.1 ;
        result = result + digit * decimal ;
        i++ ;
      } // while
    } // if

    // return stof( str ) ;
    return result * sign ;
  } // To_float()

  static bool IsDigit( char ch ) {
    return ( ch >= '0' && ch <= '9' ) ;
  } // IsDigit()

  static bool IsFloat( string str ) {
    if ( str.size() < 2 ) return false ;
    for ( int i = 0 ; i < str.size() ; i++ ) {
      if ( str[i] == '.' ) return true ;
    } // for
    
    return false ;
  } // IsFloat()

  static Token CreateNewToken( string value, TokenType type ) {
    Token newToken ;
    newToken.value = value ;
    newToken.type = type ;
    newToken.primitiveType = NONE ;
    return newToken ; 
  } // CreateNewToken()

  static Token* CloneToken( Token* token ) {
    Token* newToken = new Token() ;
    newToken->value = token->value ;
    newToken->type = token->type ;
    newToken->primitiveType = token->primitiveType ;
    return newToken ; 
  } // CreateNewToken()

  static Node* CreateNode( string name, TokenType type ) {
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
    node->address = gAddress ;

    gAddress++ ;

    return node ;
  } // CreateNode() ;

  static Node* CreateNode() {

    Node* node = new Node() ;
    node->content = NULL ;
    node->left = NULL ;
    node->right = NULL ;
    node->isConsBegin = false ;
    node->subroutineNum = 1 ;
    node->address = gAddress ;

    gAddress++ ;

    return node ;
  } // CreateNode() ;

  static Node* Go_Next_Node_And_Create( Node* cur ) {
    cur->right = CreateNode() ;
    return cur->right ;
  } // Go_Next_Node_And_Create()

  static Node* CloneTree( Node* root ) {
    
    if ( root == NULL ) return NULL ;

    // Building a new Node and clone original token
    Node* newNode = CreateNode() ;
    if ( root->content != NULL ) newNode->content = CloneToken( root->content ) ;

    // Clone Left And Right Tree
    newNode->left = CloneTree( root->left ) ;
    newNode->right = CloneTree( root->right ) ;

    return newNode ;

  } // CloneTree

  // From #<procedure +> Get +
  static string GetFunctName( string procedureFunct ) {
    if ( procedureFunct.size() < 14 ) return "\0" ;
    string funct = procedureFunct.erase( 0, 12 ) ;
    funct = funct.erase( funct.size()-1, 1 ) ;
    return funct ;
  } // GetFunctName()

} ; // SystemFunctions

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
             name == "quote"    || name == "\'"      || 
             name == "define"   ||
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
             name == "exit"     || 
             // Proj3.
             name == "let"      || name == "set!" ||
             name == "lambda"   ||
             name == "verbose"  || name == "verbose?" ) ;
  } // IsReserveWord()

} ; // TokenCheck

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

  static void PrintReserveWordBinding( string name ) {
    string binding = "#<procedure " + name + ">" ;
    cout << binding << endl ;
  } // PrintReserveWordBinding()

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

   // Print Float to consol
  static void PrintFloat( string buffer ) {

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
  string mToken_name_ ;
  Token* mCurToken_ ;
  Node* mCurNode_ ;

  Exception( ErrorType errorType ) {
    mErrorType_ = errorType ;
    mToken_name_ = "\0" ;
    mCurToken_ = NULL ;
    mCurNode_ = NULL ;
  } // Exception()

  Exception( ErrorType errorType, string token_name ) {
    mErrorType_ = errorType ;
    mToken_name_ = token_name ;
    mCurToken_ = NULL ;
    mCurNode_ = NULL ;
  } // Exception()

  Exception( ErrorType errorType, Token* token ) {
    mErrorType_ = errorType ;
    mToken_name_ = "\0" ;
    mCurToken_ = token ;
    mCurNode_ = NULL ;
  } // Exception()

  Exception( ErrorType errorType, string& token_name, Token* token ) {
    mErrorType_ = errorType ;
    mToken_name_ = token_name ;
    mCurToken_ = token ;
    mCurNode_ = NULL ;
  } // Exception()

  Exception( ErrorType errorType, string& token_name, Node* root ) {
    mErrorType_ = errorType ;
    mToken_name_ = token_name ;
    mCurToken_ = NULL ;
    mCurNode_ = root ;
  } // Exception()

  Exception( ErrorType errorType, Node* root ) {
    mErrorType_ = errorType ;
    mToken_name_ = "\0" ;
    mCurToken_ = NULL ;
    mCurNode_ = root ;
  } // Exception()

} ; // Exception


// ---------------------------ErrorHadling---------------------------------- //

class ErrorHadling {

private:

  static void PrintErrorToken( Token* token ) {
    if ( token->type == FLOAT ) Printer::PrintFloat( token->value ) ;
    else cout << token->value << endl ;
  } // PrintErrorToken()

  static void NO_MORE_INPUT_ERROR() {
    cout << "ERROR (no more input) : END-OF-FILE encountered\n" ;
  } // NO_MORE_INPUT_ERROR()

  static void NO_CLOSING_QOUTE_ERROR() {
    cout << "ERROR (no closing quote) : " 
         << "END-OF-LINE encountered at Line " << gLineNum
         << " Column " << gColumn+1 << "\n" ; // gColumn+1 means it need next token to be ')'
  } // NO_CLOSING_QOUTE_ERROR()

  static void UNRECOGNIZED_TOKEN_ERROR( string token ) {
    cout << "Unrecognized token with first char : " ;
    cout << token << endl ;
  } // UNRECOGNIZED_TOKEN_ERROR()

  static void UNEXPECTED_TOKEN_ERROR( string token ) {
    cout << "ERROR (unexpected token) :" 
         << " atom or '(' expected when token at Line " << gLineNum
         << " Column " << gColumn
         << " is >>" ;
    cout << token ;
    cout << "<<\n" ;
  } // UNEXPECTED_TOKEN_ERROR()

  static void UNEXPECTED_RIGHTBRACKET_TOKEN_ERROR( string token ) {
    cout << "ERROR (unexpected token) :" 
         << " ')' expected when token at Line " << gLineNum
         << " Column " << gColumn
         << " is >>" ;
    cout << token ;
    cout << "<<\n" ;
  } // UNEXPECTED_RIGHTBRACKET_TOKEN_ERROR()

  static void UNDEFINEDID_ERROR( string token ) {
    cout << "Undefined identifier :'" ;
    cout << token << endl ;
  } // UNDEFINEDID_ERROR()

  static void DIVID_ZERO_ERROR() {
    cout << "ERROR (division by zero) : /\n" ;
  } // DIVID_ZERO_ERROR()

  static void MEMORY_NOT_ENOUGH_ERROR() {
    cout << "Memory dosen't enough\n" ;
  } // MEMORY_NOT_ENOUGH_ERROR()

  static void UNBOUND_SYMBOL_ERROR( string token ) {
    cout << "ERROR (unbound symbol) : " ;
    cout << token << endl ;
  } // UNBOUND_SYMBOL_ERROR()

  static void NON_LIST_ERROR( Node* root ) {
    cout << "ERROR (non-list) : " ;
    Printer::PrettyPrint( root ) ;
    
  } // NON_LIST_ERROR()

  static void NON_FUNCTION_ERROR( Token* token, Node* root ) {
    cout << "ERROR (attempt to apply non-function) : " ; 
    if ( token->value != "\0" ) PrintErrorToken( token ) ;
    else Printer::PrettyPrint( root ) ;
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
    cout << "ERROR (incorrect number of arguments) : " ;
    cout << token << endl ;
  } // INCORRECT_ARGUMENTS_ERROR()

  static void INCORRECT_DEFINE_FORMAT_ERROR( Node* root ) {
    cout << "ERROR (DEFINE format) : " ;
    Printer::PrettyPrint( root ) ;
  } // INCORRECT_DEFINE_FORMAT_ERROR()

  static void INCORRECT_ARGUMENT_TYPE_ERROR( string func_name, Token* token, Node* root ) {
    cout << "ERROR (" << func_name << " with incorrect argument type) : " ;
    if ( token->value != "\0" ) PrintErrorToken( token ) ;
    else Printer::PrettyPrint( root ) ;
  } // INCORRECT_ARGUMENT_TYPE_ERROR()

  static void INCORRECT_COND_FORMAT_ERROR( Node* root ) {
    cout << "ERROR (COND format) : " ;
    Printer::PrettyPrint( root ) ;
  } // INCORRECT_COND_FORMAT_ERROR()

  static void INCORRECT_LIST_FORMAT_ERROR( Node* root ) {
    cout << "ERROR (LIST format) : " ;
    Printer::PrettyPrint( root ) ;
  } // INCORRECT_LIST_FORMAT_ERROR()

  static void INCORRECT_LET_FORMAT_ERROR() {
    cout << "ERROR (let format)\n" ;
    // Printer::PrettyPrint( root ) ;
  } // INCORRECT_LET_FORMAT_ERROR()

  static void INCORRECT_LAMBDA_FORMAT_ERROR() {
    cout << "ERROR (lambda format)\n" ;
    // Printer::PrettyPrint( root ) ;
  } // INCORRECT_LAMBDA_FORMAT_ERROR()

  static void NO_RETURN_VALUE_ERROR( Node* root ) {
    cout << "ERROR (no return value) : " ;
    Printer::PrettyPrint( root ) ;
  } // NO_RETURN_VALUE_ERROR()

  static void TESTING_ERROR( string token ) {
    cout << "Testing bug with token : " ;
    cout << token << endl ;
  } // TESTING_ERROR()

public:

  static void ErrorMessage( ErrorType type, string token_name, Token* token, Node* root ) {
    if ( type == NO_MORE_INPUT ) NO_MORE_INPUT_ERROR() ;
    else if ( type == NO_CLOSING_QUOTE ) NO_CLOSING_QOUTE_ERROR() ;
    else if ( type == UNRECOGNIZED_TOKEN ) UNRECOGNIZED_TOKEN_ERROR( token_name ) ;
    else if ( type == UNEXPECTED_TOKEN ) UNEXPECTED_TOKEN_ERROR( token_name ) ;
    else if ( type == UNEXPECTED_RIGHTBRACKET_TOKEN ) UNEXPECTED_RIGHTBRACKET_TOKEN_ERROR( token_name ) ;
    else if ( type == UNDEFINEDID ) UNDEFINEDID_ERROR( token_name ) ;
    else if ( type == DIVID_ZERO ) DIVID_ZERO_ERROR() ;
    else if ( type == MEMORY_NOT_ENOUGH ) MEMORY_NOT_ENOUGH_ERROR() ;
    else if ( type == UNBOUND_SYMBOL ) UNBOUND_SYMBOL_ERROR( token_name ) ;
    else if ( type == NON_LIST ) NON_LIST_ERROR( root ) ;
    else if ( type == NON_FUNCTION ) NON_FUNCTION_ERROR( token, root ) ;
    else if ( type == LEVEL_OF_EXIT ) LEVEL_OF_EXIT_ERROR() ;
    else if ( type == LEVEL_OF_DEFINE ) LEVEL_OF_DEFINE_ERROR() ;
    else if ( type == LEVEL_OF_CLEAN_ENVIRONMENT ) LEVEL_OF_CLEAN_ENVIRONMENT_ERROR() ;
    else if ( type == INCORRECT_ARGUMENTS ) INCORRECT_ARGUMENTS_ERROR( token_name ) ;
    else if ( type == INCORRECT_DEFINE_FORMAT ) INCORRECT_DEFINE_FORMAT_ERROR( root ) ;
    else if ( type == INCORRECT_ARGUMENT_TYPE ) INCORRECT_ARGUMENT_TYPE_ERROR( token_name, token, root ) ;
    else if ( type == INCORRECT_COND_FORMAT ) INCORRECT_COND_FORMAT_ERROR( root ) ;
    else if ( type == INCORRECT_LIST_FORMAT ) INCORRECT_LIST_FORMAT_ERROR( root ) ;
    else if ( type == INCORRECT_LET_FORMAT ) INCORRECT_LET_FORMAT_ERROR() ;
    else if ( type == INCORRECT_LAMBDA_FORMAT ) INCORRECT_LAMBDA_FORMAT_ERROR() ;
    else if ( type == NO_RETURN_VALUE ) NO_RETURN_VALUE_ERROR( root ) ;
    else if ( type == TESTING ) TESTING_ERROR( token_name ) ;

  } // ErrorMessage()

} ; // ErrorHadling


// ---------------------------DataBase------------------------------------ //

class Hash {

  public:
  static int Eval_Key( string str ) {
    int hashkey = 0 ;

    for ( int i = 0 ; i < str.size() ; i++ ) {
      hashkey = ( hashkey + str[i] ) % MAX_MEMORY_SIZE ;
    } // for

    return hashkey ;
  } // Eval_Key()

} ; // Hash


// ---------------------------Symbol Table------------------------------------ //

// Symbol Table using hash function to store name and value.
class SymbolTable {
  
private:
  vector<Symbol*> *gSymbolTable ;

  // Variable Symbol
  Symbol* CreateSymbol( string name, Node* binding ) {
    Symbol* newSymbol = new Symbol() ;
    newSymbol->name = name ;
    newSymbol->binding = binding ;
    newSymbol->parameter = NULL ;
    newSymbol->isFunction = false ;
    newSymbol->next = NULL ;
    
    return newSymbol ;
  } // CreateSymbol()

  // Function Symbol
  Symbol* CreateSymbol( string name, Node* parameter, Node*expression, Node* binding ) {
    Symbol* newSymbol = new Symbol() ;
    newSymbol->name = name ;
    newSymbol->binding = binding ;
    newSymbol->parameter = parameter ;
    newSymbol->expression = expression ;
    newSymbol->isFunction = true ;
    newSymbol->next = NULL ;

    return newSymbol ;
  } // CreateSymbol()

  // Evaluateing key and dealing with collision
  // case1: if there is same name symbol, return it's key
  // case2: if there is no same name symbol, return the key 
  // which is empty ( NULL or name == "\0" ) and nearest to original hash key
  int GetKey( string name ) {
    int key = Hash::Eval_Key( name ) ;
    int empty_node_Key = -1 ;
    int count = 0 ;
    Symbol* cur = gSymbolTable->at( key ) ;
    
    // if same name, return the key
    if ( cur != NULL && cur->name == name ) return key ;

    // if the not NULL means the key had used before
    while ( cur != NULL ) {
      if ( cur->name == name ) return key ;
      else if ( cur->name == "\0" && empty_node_Key == -1 ) empty_node_Key = key ;

      key++ ;
      key %= MAX_MEMORY_SIZE ; // avoid out of size ;
      cur = gSymbolTable->at( key ) ;

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

  // Return #<procedure reserveWord>
  Node* GetReserveWordBinding( string name ) {
    string binding ;
    TokenType type = SYMBOL ;
    if ( name == "\'" || name == "quote" ) type = QUOTE ;
    binding = "#<procedure " + name + ">" ;
    return SystemFunctions::CreateNode( binding, type ) ;
  } // GetReserveWordBinding()

  // Return last define symbol
  Symbol* GetSymbol( int key ) {
    Symbol* cur = gSymbolTable->at( key ) ;
    while ( cur->next != NULL ) cur = cur->next ;
    return cur ;
  } // GetSymbol()

public:
  SymbolTable() {
    gSymbolTable = new vector<Symbol*>( MAX_MEMORY_SIZE ) ;
  } // SymbolTable()

  Node* GetBinding( string name ) {

    // Return Reserved word binding
    if ( TokenChecker::IsReserveWord( name ) )
      return GetReserveWordBinding( name ) ;

    int key = GetKey( name ) ;
    if ( gSymbolTable->at( key ) == NULL ) throw Exception( UNBOUND_SYMBOL, name ) ;

    // Get the last symbol
    Symbol* symbol = GetSymbol( key ) ;
    
    return symbol->binding ;
  } // GetBinding()

  bool IsFunctionSymbol( string name ) {
    int key = GetKey( name ) ;
    if ( gSymbolTable->at( key ) == NULL ) return false ;

    Symbol* symbol = GetSymbol( key ) ;
    return ( symbol->isFunction ) ;
  } // IsFunctionSymbol()

  Node* GetParameter( string name ) {

    int key = GetKey( name ) ;
    if ( gSymbolTable->at( key ) == NULL ) throw Exception( UNBOUND_SYMBOL, name ) ;

    // Get the last symbol
    Symbol* symbol = GetSymbol( key ) ;
    
    return symbol->parameter ;
  } // GetParameter()

  Node* GetExpression( string name ) {

    int key = GetKey( name ) ;
    if ( gSymbolTable->at( key ) == NULL ) throw Exception( UNBOUND_SYMBOL, name ) ;

    // Get the last symbol
    Symbol* symbol = GetSymbol( key ) ;
    
    return symbol->expression ;
  } // GetParameter()

  // if symbol exit, return true
  bool Find( string name ) {
    int key = GetKey( name ) ;
    return ( gSymbolTable->at( key ) == NULL ) ;
  } // Find()

  // Insert <Symbol, Parameter, Binding> to symble table
  // If there is same name sybol, append it
  void Insert( string name, Node* parameter, Node* expression, Node* binding ) {
    int key = GetKey( name ) ;
    Symbol* cur = gSymbolTable->at( key ) ;

    // ---------- STEP1: Create a new symbol ---------- //
    Symbol* newSymbol = NULL ;
    if ( parameter != NULL || expression != NULL )
      newSymbol = CreateSymbol( name, parameter, expression, binding ) ;
    else newSymbol = CreateSymbol( name, binding ) ;
    // cout << name << ":" << key << endl ; // debug

    // ---------- STEP2: Insert to symbol table ---------- //

    // case1: No collision
    if ( cur == NULL ) gSymbolTable->at( key ) = newSymbol ;
    // case2: If the node dosen't free up, replace it
    else if ( cur->name == "\0" ) {
      // delete gSymbolTable->at( key ) ;
      gSymbolTable->at( key ) = newSymbol ;
    } // else if
    // case3: If collision and the symbol has same name
    else if ( cur->name == name ) {
      // note: notice define repeatly problem
      // go to the last not NULL symbol
      while ( cur->next != NULL ) cur = cur->next ;
      // append the new same name symbol to last
      cur->next = newSymbol ;
    } // else if

    // No more space can add
    else throw Exception( MEMORY_NOT_ENOUGH ) ;
    
  } // Insert()

  // Only Delete the content but not to free up space, so the pointer would be exist
  void Delete( string name ) {

    int key = GetKey( name ) ;

    if ( gSymbolTable->at( key ) == NULL ) return ;

    gSymbolTable->at( key )->name = "\0" ;
    // free the tree memory
    FreeUpValue( gSymbolTable->at( key )->binding ) ;
    gSymbolTable->at( key )->binding = NULL ;

    // TODO next Case
  } // Delete()

  // clear symbol table
  void Clear() {
    gSymbolTable->clear() ;
    gSymbolTable = new vector<Symbol*>( MAX_MEMORY_SIZE ) ;
  } // Clear()

} ;

SymbolTable gSymbolTable ;


// ---------------------------Function Segment------------------------------------ //

class FunctionSegment {

private:
  string mFuntionName_ ;
  map< string, Node* >* mLocalSymbolTable_ ;
  Node* mParameter_ ;
  Node* mExpression_ ;
  Node* mReturn_ ;

public:

  // No name Function
  FunctionSegment() {
    // create a new local symbol table to save parameter
    mLocalSymbolTable_ = new map< string, Node* >() ;
  } // FunctionSegment()

  FunctionSegment( string func_name ) {
    // create a new local symbol table to save parameter
    mLocalSymbolTable_ = new map< string, Node* >() ;
    // Go to global symbol table and get function expression
    mExpression_ = gSymbolTable.GetExpression( func_name ) ;
    mParameter_ = gSymbolTable.GetParameter( func_name ) ;
  } // FunctionSegment()
  
  ~FunctionSegment() {
    delete mLocalSymbolTable_ ;
  } // ~FunctionSegment()

  string GetName() {
    return mFuntionName_ ;
  } // SetName()

  // return NULL or parameters
  Node* GetParameter() {
    return mParameter_ ;
  } // GetParameter()

  // return NULL or parameters
  Node* GetExpression() {
    return mExpression_ ;
  } // GetExpression()

  // As calling function, bypass parameter
  void ByPassParameter( string para_name, Node* binding ) {
    mLocalSymbolTable_->insert( { para_name, binding } ) ;
  } // ByPassParameter()

  Node* GetParameterBinding( string para_name ) {
    return mLocalSymbolTable_->at( para_name ) ;
  } // GetParameterBinding()

  bool FindParameter( string para_name ) {
    return ( mLocalSymbolTable_->count( para_name ) > 0 ) ;
  } // FindParameter()

} ; // FunctionSegment


// ---------------------------Call Stack------------------------------------ //

class CallStack {

private:
  vector<FunctionSegment*> *mFunctionCallStack_ ;

  FunctionSegment* CreateFunctionSegment() {
    FunctionSegment* function_seg = new FunctionSegment() ;
    return function_seg ; 
  } // CreateFunctionSegment()

  FunctionSegment* CreateFunctionSegment( string name ) {
    FunctionSegment* function_seg = new FunctionSegment( name ) ;
    return function_seg ; 
  } // CreateFunctionSegment()

public:
  CallStack() {
    mFunctionCallStack_ = new vector<FunctionSegment*>() ;
  } // CallStack()

  ~CallStack() {
    delete mFunctionCallStack_ ;
  } // ~CallStack()

  void Push() {
    mFunctionCallStack_->push_back( CreateFunctionSegment() ) ;
  } // Push()

  void Push( string func_name ) {
    mFunctionCallStack_->push_back( CreateFunctionSegment( func_name ) ) ;
  } // Push()

  void Push( string func_name, Node* bypass_root ) {
    // ---------- STEP1: Push a Func Segment to stack ---------- //
    mFunctionCallStack_->push_back( CreateFunctionSegment( func_name ) ) ;
    // Get current calling function
    FunctionSegment* calling_function = Top() ;

    // ---------- STEP2: ByPassing Parameter ---------- //
    Node* bypass_cur = bypass_root ;
    Node* funcPara_cur = calling_function->GetParameter() ;

    // No Parameter
    if ( bypass_cur == NULL && funcPara_cur->content->type == NIL ) return ;

    // Get all parameter and bypass
    while ( funcPara_cur != NULL && funcPara_cur->content == NULL ) {
      
      // Set parameter name and bypass
      string parameter_name = funcPara_cur->left->content->value ;
      Node* parameter_binding_value = bypass_cur->left ;

      // if ( parameter_binding_value == NULL ) throw Exception( TESTING, "Push: Binding Null" ) ;
      // binding bypassing value
      calling_function->ByPassParameter( parameter_name, parameter_binding_value ) ;

      bypass_cur = bypass_cur->right ;
      funcPara_cur = funcPara_cur->right ;
    } // while
  } // Push()

  FunctionSegment* Top() {
    if ( mFunctionCallStack_->empty() ) return NULL ;
    return mFunctionCallStack_->back() ;
  } // Top()

  void Pop() {
    if ( ! mFunctionCallStack_->empty() )
      mFunctionCallStack_->erase( mFunctionCallStack_->end() - 1 ) ;
  } // Pop()

  bool IsEmpty() {
    return mFunctionCallStack_->empty() ;
  } // IsEmpty()
} ; // Stack


// ---------------------------Processing Stage--------------------------- //
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

  vector<string>* mFunc_Para_Name_ ;

  // Return Is Equal: + *- * / > >= < <= =
  bool IsBasicArithmeticOperator( string op ) {
    return ( op == "+"   || op == "-"  ||
             op == "*"   || op == "/"  ||
             op == ">"   || op == ">=" ||
             op == "<"   || op == "<=" ||
             op == "="   ) ;
  } // IsBasicArithmeticOperator()

  // Return Is Equal: string-append string>? string<? string=?
  bool IsBasicStringOperator( string op ) {
    return ( op == "string-append" ||
             op == "string>?"      ||
             op == "string<?"      ||
             op == "string=?" ) ;
  } // IsBasicStringOperator()

  bool IsCorrectArithmeticType( TokenType arg_type ) {
    return ( arg_type == INT || arg_type == FLOAT ) ;
  } // IsCorrectArithmeticType()

  bool IsCorrectStringType( TokenType arg_type ) {
    return ( arg_type == STRING ) ;
  } // IsCorrectStringType()

  int Get_count( Node* root ) {
    Node* cur = root ;
    int count = 0 ;

    while ( cur != NULL && cur->content == NULL ) {
      count++ ;
      cur = cur->right ;
    } // while 

    // Pared node not nil
    // if ( cur != NULL && cur->content != NULL && cur->content->type != NIL ) 
    //   throw Exception( TESTING ) ;
    return count ;

  } // Get_count()

  bool IsCondListFormatCorrect( Node* cond_Root ) {
    Node* cur = cond_Root ;

    // Through all nodes
    while ( cur != NULL ) {
      // Check all left node exist and doesn't has paired node
      if ( cur->left == NULL || cur->content != NULL ) return false ;
      cur = cur->right ;
    } // while

    return true ;
  } // IsCondListFormatCorrect()

  bool IsNameRepeat( string name ) {
    for ( int i = 0 ; i < mFunc_Para_Name_->size() ; i++ ) {
      if ( mFunc_Para_Name_->at( i ) == name ) return true ;
    } // for

    return false ;
  } // IsNameRepeat

public:
  SemanticsAnalyzer() {
    mFunc_Para_Name_ = new vector<string>() ;
  } // SemanticsAnalyzer()

  void Check_Basic_Arthmatic_Argument( string function_name, Node* argument ) {
    // Check if there still has list, even though s-expression had expanded
    if ( argument->content == NULL )
      throw Exception( INCORRECT_ARGUMENT_TYPE, function_name, argument ) ;
    // Cehck Type is Correct
    else if ( ! IsCorrectArithmeticType( argument->content->type ) )
      throw Exception( INCORRECT_ARGUMENT_TYPE, function_name, argument->content ) ;
  } // Check_Basic_Arthmatic_Argument()

  void Check_Divid_Zero( string function_name, Node* argument ) {
    // Check if divid zero
    if ( argument->content->value == "0" && function_name == "/" ) 
      throw Exception( DIVID_ZERO ) ;
  } // Check_Divid_Zero()

  void Check_Basic_Comparing_Argument( string function_name, Node* argument ) {
    Check_Basic_Arthmatic_Argument( function_name, argument ) ;
  } // Check_Basic_Comparing_Argument()

  void Check_Basic_String_Argument( string function_name, Node* argument ) {
    // Check if there still has list, even though s-expression had expanded
    if ( argument->content == NULL )
      throw Exception( INCORRECT_ARGUMENT_TYPE, function_name, argument ) ;
    // Cehck Type is Correct
    else if ( ! IsCorrectStringType( argument->content->type ) )
      throw Exception( INCORRECT_ARGUMENT_TYPE, function_name, argument->content ) ;
  } // Check_Basic_String_Argument()

  void Check_Basic_Boolean_Argument( string function_name, Node* argument ) {
    // Check if there still has list, even though s-expression had expanded
    if ( argument->content == NULL && function_name != "not" )
      throw Exception( INCORRECT_ARGUMENT_TYPE, function_name, argument ) ;
  } // Check_Basic_Boolean_Argument()

  void Check_No_Pared( Node* root ) {
    Node* cur = root ;
    while ( cur->right != NULL ) cur = cur->right ;
    if ( cur != NULL && cur->content != NULL ) throw Exception( NON_LIST, root ) ;
  } // Check_No_Pared()

  void Check_Let_Format_Correct( Node* let_first_argument ) {
    mFunc_Para_Name_->clear() ; // Initialize
    Node* cur = let_first_argument->left ;
    Check_Let_S_Expression_Format_Correct( cur ) ;

    // Check each definition format
    while ( cur != NULL && cur->content == NULL ) {
      Check_Let_Arg_Define_Format_Correct( cur->left ) ;
      if ( IsNameRepeat( cur->left->left->content->value ) )
        throw Exception( INCORRECT_LET_FORMAT ) ;
      mFunc_Para_Name_->push_back( cur->left->left->content->value ) ;
      cur = cur->right ;
    } // while
    
    mFunc_Para_Name_->clear() ; // Initialize
  } // Check_Let_Format_Correct()

  // Check Format: (( ... ) ( ... ) ) or ()
  void Check_Let_S_Expression_Format_Correct( Node* argument ) {
    // Error Format: ( let (5) ... )
    if ( argument->content != NULL && argument->content->type != NIL )
      throw Exception( INCORRECT_LET_FORMAT ) ;
  } // Check_Let_S_Expression_Format_Correct()

  // Check Format: ( Symbol S-Expression )
  void Check_Let_Arg_Define_Format_Correct( Node* arg_def ) {
    // Error Format: ( x 1 2 )
    if ( ! Is_ArgCount_Equal_To( arg_def, 2 ) ) 
      throw Exception( INCORRECT_LET_FORMAT ) ;
    // Error Format: Runtime Error
    else if ( arg_def->left == NULL )
      throw Exception( INCORRECT_LET_FORMAT ) ;
    // Error Format: ( '( 1 ) 2 )
    else if ( arg_def->left->content == NULL )
      throw Exception( INCORRECT_LET_FORMAT ) ;
    // Error Format: ( 1 2 )
    else if ( arg_def->left->content->type != SYMBOL )
      throw Exception( INCORRECT_LET_FORMAT ) ;
    // Error Format: ( cons 2 ) can't define reserve word
    else if ( TokenChecker::IsReserveWord( arg_def->left->content->value ) )
      throw Exception( INCORRECT_LET_FORMAT ) ;

  } // Check_Let_Arg_Define_Format_Correct()

  // Check Format: ( x y z ) or ()
  void Check_Lambda_Format_Correct( Node* lambda_first_argument ) {
    mFunc_Para_Name_->clear() ; // Initialize
    Node* cur = lambda_first_argument->left ;
    
    // Check Format: nil
    if ( cur->content != NULL && cur->content->type != NIL ) throw Exception( INCORRECT_LAMBDA_FORMAT ) ;
    // Check Format: ( symbols )
    else if ( cur->content == NULL ) {

      // Check each definition format
      while ( cur != NULL ) {
        Node* parameter = cur->left ;
        // if left
        if ( parameter == NULL          || 
             parameter->content == NULL ||
             parameter->content->type != SYMBOL ) throw Exception( INCORRECT_LAMBDA_FORMAT ) ;
        
        if ( IsNameRepeat( parameter->content->value ) )
          throw Exception( INCORRECT_LAMBDA_FORMAT ) ;
        mFunc_Para_Name_->push_back( parameter->content->value ) ;
        cur = cur->right ;
      } // while

    } // else if
    
    mFunc_Para_Name_->clear() ; // Initialize
  } // Check_Lambda_Format_Correct()

  void Check_Lambda_Call_Function_ByPass_Format( Node* func_para, Node* bypass ) {
    
    // cout << "Function Parameter:\n" ;
    // Printer::PrettyPrint( func_para ) ;
    // cout << "ByPass Parameter:\n" ;
    // Printer::PrettyPrint( bypass ) ;  

    if ( func_para == NULL )
      throw Exception( TESTING, "Call Function Check: func_parameter is null" ) ;
    // ( ( lambda () 1 ) )
    else if ( bypass == NULL && func_para->content != NULL && func_para->content->type == NIL )
      return ;
    else if ( ! Is_ArgCount_Equal_To( func_para, bypass ) )
      throw Exception( INCORRECT_ARGUMENTS, "lambda expression" ) ;

  } // Check_Lambda_Call_Function_ByPass_Format()

  void Check_Call_Function_ByPass_Format( Node* func_para, Node* bypass ) {
    
    // cout << "Function Parameter:\n" ;
    // Printer::PrettyPrint( func_para ) ;
    // cout << "ByPass Parameter:\n" ;
    // Printer::PrettyPrint( bypass ) ;  

    if ( func_para == NULL )
      throw Exception( TESTING, "Call Function Check: func_parameter is null" ) ;
    // ( ( lambda () 1 ) )
    else if ( bypass == NULL && func_para->content != NULL && func_para->content->type == NIL )
      return ;
    else if ( ! Is_ArgCount_Equal_To( func_para, bypass ) )
      throw Exception( INCORRECT_ARGUMENTS, "expression" ) ;

  } // Check_Lambda_Call_Function_ByPass_Format()

  // Check the tree arguments is equal to ?
  bool Is_ArgCount_Equal_To( Node* arg_root, int count ) {
    int arg_count = Get_count( arg_root ) ;
    return ( arg_count == count ) ;
  } // Is_ArgCount_Equal_To()

  bool Is_ArgCount_Equal_To( Node* arg_root, Node* arg_root2 ) {
    int arg_count = Get_count( arg_root ) ;
    int arg_count2 = Get_count( arg_root2 ) ;
    return ( arg_count == arg_count2 ) ;
  } // Is_ArgCount_Equal_To()

  bool Is_ArgCount_Equal_To( Node* arg_root, int count1, int count2 ) {
    int arg_count = Get_count( arg_root ) ;
    return ( arg_count == count1 || arg_count == count2 ) ;
  } // Is_ArgCount_Equal_To()
  
  bool Is_ArgCount_BiggerEqual_Than( Node* arg_root, int count ) {
    int arg_count = Get_count( arg_root ) ;
    return ( arg_count >= count ) ;
  } // Is_ArgCount_BiggerEqual_Than()

  void Check_Cond_Format( Node* root ) {
    Node* cur = root->right ;

    // check argument count correct
    if ( ! Is_ArgCount_BiggerEqual_Than( cur, 1 ) )
      throw Exception( INCORRECT_COND_FORMAT, root ) ;
    
    // Through all nodes: check all condition list
    while ( cur != NULL ) {
      // Get cond list root
      Node* cond_root = cur->left ;

      // if first argument not a list ERROR
      if ( cond_root == NULL ) throw Exception( INCORRECT_COND_FORMAT, root ) ;
      else if ( cond_root->content != NULL || cond_root->left == NULL )
        throw Exception( INCORRECT_COND_FORMAT, root ) ;
      // if condition only has one argument
      else if ( cond_root->right == NULL ) throw Exception( INCORRECT_COND_FORMAT, root ) ;

      if ( ! IsCondListFormatCorrect( cond_root ) )
        throw Exception( INCORRECT_COND_FORMAT, root ) ;
      
      cur = cur->right ;
    } // while

  } // Check_Cond_Format()

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
    oneNode->address = gAddress ;
    oneNode->isConsBegin = false ;

    gAddress ++ ;
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
  
  SemanticsAnalyzer mSemanticsAnalyzer_ ;
  CallStack mCallStack_ ;
  Symbol* mLambda_Func_ ;
  vector<Token> *mCommand_ ;
  int mLevel_ ;
  bool mIsQuit ;

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
    node->address = gAddress ;

    gAddress++ ;

    return node ;
  } // CreateNode() ;

  string CreateFunctionName( string name ) {
    string funct = "#<procedure " + name + ">" ;
    return funct ;
  } // CreateFunctionName()

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

  // Evaluate: + - * /
  Node* BasicArithmaticCalculate( string op, Node* arg_root ) {

    // root only have parameters, not including function name
    Node* cur = arg_root ;
    Node* eval_argument = Evaluate( GetArgument( arg_root ) ) ;
    // Semantics Analyzing
    mSemanticsAnalyzer_.Check_Basic_Arthmatic_Argument( op, eval_argument ) ;

    // Get the first number
    float result = SystemFunctions::To_float( eval_argument->content->value ) ;

    // Type Check
    TokenType type = INT ;
    if ( eval_argument->content->type == FLOAT ) type = FLOAT ;

    // Continue to get next number ( started with second argument )
    while ( cur->right != NULL ) {
      cur = cur->right ;

      // Get next evaluated argument
      eval_argument = Evaluate( GetArgument( cur ) ) ;

      // Semantics Analyzing
      mSemanticsAnalyzer_.Check_Basic_Arthmatic_Argument( op, eval_argument ) ;
      mSemanticsAnalyzer_.Check_Divid_Zero( op, eval_argument ) ;
      
      // Check Type is Float or Int
      if ( eval_argument->content->type == FLOAT ) type = FLOAT ;

      // Calculating
      if ( op == "+" ) result += SystemFunctions::To_float( eval_argument->content->value ) ;
      else if ( op == "-" ) result -= SystemFunctions::To_float( eval_argument->content->value ) ;
      else if ( op == "*" ) result *= SystemFunctions::To_float( eval_argument->content->value ) ;
      else if ( op == "/" ) result /= SystemFunctions::To_float( eval_argument->content->value ) ;
      
    } // while

    if ( type == INT ) result = trunc( result ) ;

    return CreateNode( SystemFunctions::To_string( result ), type ) ;
  } // BasicArithmaticCalculate()

  // Evaluate: > >= < <= =
  Node* BasicComparing( string op, Node* arg_root ) {

    // root only have parameters, not including function name
    Node* cur = arg_root ;
    Node* eval_argument = Evaluate( GetArgument( arg_root ) ) ;
    mSemanticsAnalyzer_.Check_Basic_Comparing_Argument( op, eval_argument ) ;

    // Get the first number
    float comparingNum = SystemFunctions::To_float( eval_argument->content->value ) ;
    TokenType type = T ;

    // Continue to get next number ( started with second argument )
    while ( cur->right != NULL ) {
      cur = cur->right ;

      // Get next evaluated argument
      eval_argument = Evaluate( GetArgument( cur ) ) ;
      mSemanticsAnalyzer_.Check_Basic_Comparing_Argument( op, eval_argument ) ;

      float argument = SystemFunctions::To_float( eval_argument->content->value ) ;
      
      // Calculating
      if ( op == ">" ) {
        if ( comparingNum <= argument ) type = NIL ;
      } // if
      else if ( op == ">=" ) {
        if ( comparingNum < argument ) type = NIL ;
      } // else if
      else if ( op == "<" ) {
        if ( comparingNum >= argument ) type = NIL ;
      } // else if
      else if ( op == "<=" ) {
        if ( comparingNum > argument ) type = NIL ;
      } // else if
      else if ( op == "=" ) {
        if ( comparingNum != argument ) type = NIL ;
      } // else if

      comparingNum = argument ;
    } // while

    string result = "#t" ;
    if ( type == NIL ) result = "nil" ;

    return CreateNode( result, type ) ;
  } // BasicComparing()

  // Evaluate: string-append string>? string<? string=?
  Node* BasicString_Operate( string op, Node* arg_root ) {

    // root only have parameters, not including function name
    Node* cur = arg_root ;
    Node* eval_argument = Evaluate( GetArgument( arg_root ) ) ;
    mSemanticsAnalyzer_.Check_Basic_String_Argument( op, eval_argument ) ;

    // Get the first number
    string first_arg = eval_argument->content->value ;
    string result = "#t" ;
    TokenType type = T ;

    // Continue to get next number
    while ( cur->right != NULL ) {
      cur = cur->right ;

      // Get next evaluated argument
      Node* eval_argument = Evaluate( GetArgument( cur ) ) ;
      mSemanticsAnalyzer_.Check_Basic_String_Argument( op, eval_argument ) ;

      string argument = eval_argument->content->value ;
      
      // Calculating
      if ( op == "string-append" ) {
        first_arg = AppendString( first_arg, argument ) ;
        type = STRING ;
      } // if
      else if ( op == "string>?" ) {
        if ( first_arg.compare( argument ) <= 0 ) type = NIL ;
        first_arg = argument ;
      } // else if
      else if ( op == "string<?" ) {
        if ( first_arg.compare( argument ) >= 0 ) type = NIL ;
        first_arg = argument ;
      } // else if
      else if ( op == "string=?" ) {
        if ( first_arg.compare( argument ) != 0 ) type = NIL ;
        first_arg = argument ;
      } // else if
      
    } // while
    
    if ( type == STRING ) result = first_arg ;
    else if ( type == NIL ) result = "nil" ;

    return CreateNode( result, type ) ;
  } // BasicString_Operate()

  string AppendString( string& str1, string& str2 ) {
    str1.erase( str1.size()-1, 1 ) ;
    str2.erase( 0, 1 ) ;
    return str1 += str2 ;
  } // AppendString()

  Node* BasicAND( Node* arg_root ) {
    
    // root only have parameters, not including function name
    Node* cur = arg_root ;
    Node* result = Evaluate( GetArgument( arg_root ) ) ;

    // Continue to get next value
    while ( cur != NULL ) {
      
      // Get Evaluated Argument 
      Node* eval_argument = Evaluate( GetArgument( cur ) ) ;
      // mSemanticsAnalyzer_.Check_Basic_Boolean_Argument( "and", eval_argument ) ;

      if ( eval_argument->content != NULL ) {
        TokenType arg_type = eval_argument->content->type ;
        
        // Boolean Operating
        if ( arg_type == NIL ) return CreateNode( "nil", NIL ) ;
      } // if

      // If the current node is last
      if ( cur->right == NULL ) result = eval_argument ;
      
      // Go next argument
      cur = cur->right ;
    } // while

    return result ;

  } // BasicAND()

  Node* BasicOR( Node* arg_root ) {
    
    // root only have parameters, not including function name
    Node* cur = arg_root ;
    Node* result = Evaluate( GetArgument( arg_root ) ) ;

    // Continue to get next value
    while ( cur != NULL ) {
      
      // Get Evaluated Argument 
      Node* eval_argument = Evaluate( GetArgument( cur ) ) ;
      // mSemanticsAnalyzer_.Check_Basic_Boolean_Argument( "or", eval_argument ) ;

      if ( eval_argument->content != NULL ) {
        TokenType arg_type = eval_argument->content->type ;
        
        // Boolean Operating
        if ( arg_type != NIL ) return eval_argument ;
      } // if

      if ( cur->right == NULL ) result = eval_argument ;

      // Go next argument
      cur = cur->right ;
    } // while

    return result ;

  } // BasicOR()

  Node* BasicNOT( Node* arg_root ) {
    
    // Get Evaluated Argument 
    Node* eval_argument = Evaluate( GetArgument( arg_root ) ) ;
    mSemanticsAnalyzer_.Check_Basic_Boolean_Argument( "not", eval_argument ) ;

    if ( eval_argument->content == NULL ) return CreateNode( "nil", NIL ) ;
    TokenType type = eval_argument->content->type ;

    if ( type == NIL ) return CreateNode( "#t", T ) ;

    return CreateNode( "nil", NIL )  ;

  } // BasicNOT()

  Node* Eqv( Node* arg_root ) {
    
    // Get Arguments
    Node* first_arg = Evaluate( GetArgument( arg_root ) ) ;
    Node* second_arg = Evaluate( GetArgument( arg_root->right ) ) ;

    // case1: ( eqv? 3 3 ) same atom and same value
    if ( first_arg->content != NULL && second_arg->content != NULL ) {
      if ( TokenChecker::IsAtom( first_arg->content->type )  &&
           TokenChecker::IsAtom( second_arg->content->type ) &&
           first_arg->content->value == second_arg->content->value &&
           first_arg->content->type != STRING &&
           second_arg->content->type != STRING )
        return CreateNode( "#t", T ) ;

      if ( first_arg->address == second_arg->address ) return CreateNode( "#t", T ) ;
    } // if

    // case2: ( eqv? a a ) same memory space
    else if ( first_arg->content == NULL && second_arg->content == NULL ) {
      if ( first_arg->address == second_arg->address ) return CreateNode( "#t", T ) ;
    } // else if

    return CreateNode( "nil", NIL ) ;
  } // Eqv()

  Node* Equal( Node* arg_root ) {
    // Get Arguments
    Node* first_arg = Evaluate( GetArgument( arg_root ) ) ;
    Node* second_arg = Evaluate( GetArgument( arg_root->right ) ) ;

    // case1: ( equal? 3 3 ) same value
    if ( first_arg->content != NULL && second_arg->content != NULL ) {
      if ( first_arg->content->value == second_arg->content->value ) return CreateNode( "#t", T ) ;
    } // if

    // case2: ( equal? '(1 2) '(1 2) ) same list
    else if ( first_arg->content == NULL && second_arg->content == NULL ) {
      if ( IsSameList( first_arg, second_arg ) ) return CreateNode( "#t", T ) ;
    } // else if

    return CreateNode( "nil", NIL ) ;
  } // Equal()

  bool IsSameList( Node* root1, Node* root2 ) {
    
    // terminal condition
    if ( root1 == NULL && root2 == NULL ) return true ;
    else if ( root1 == NULL && root2 != NULL ) return false ;
    else if ( root1 != NULL && root2 == NULL ) return false ;

    if ( ! IsSameList( root1->left, root2->left ) ) return false ;

    // node has value but not same
    if ( root1->content != NULL && root2->content != NULL ) {
      if ( root1->content->value != root2->content->value ) return false ;
    } // if
    
    if ( root1->content == NULL && root2->content != NULL ) return false ;
    if ( root1->content != NULL && root2->content == NULL ) return false ;

    if ( ! IsSameList( root1->right, root2->right ) ) return false ;

    return true ;

  } // IsSameList()

  Node* Eval_If( Node* root ) {
    Node* arg_root = root->right ;
    Node* condition = Evaluate( arg_root->left ) ;
    Node* statement1 = arg_root->right->left ;
    Node* statement2 = NULL ;

    // if condition is a pared or a list
    if ( condition->content == NULL ) return Evaluate( statement1 ) ;

    // if has false condition statement 
    if ( arg_root->right->right != NULL )
      statement2 = arg_root->right->right->left ;

    // false condition doesn't has statement
    if ( condition->content->type == NIL && statement2 == NULL )
      throw Exception( NO_RETURN_VALUE, root ) ;

    return ( condition->content->type != NIL ) ? ( Evaluate( statement1 ) ) : ( Evaluate( statement2 ) ) ;
  } // Eval_If()

  // Evaluate Cond Function
  Node* Eval_Cond( Node* root ) {
    
    // Init and set up first condition node
    Node* arg_cur = root->right ; // arguments root
    Node* condition = NULL ;
    int cond_count = 0 ;

    // correct format ( cond ( (condition) statement ) )
    while ( arg_cur != NULL ) {
      Node* cond_root = arg_cur->left ; // condition root
      cond_count++ ;

      // check the last condition if is else statement
      if ( arg_cur->right == NULL && 
           cond_count > 1     &&
           cond_root->left->content != NULL &&
           cond_root->left->content->value == "else" ) {
        
        // if ( cond_root->right == NULL ) throw Exception( INCORRECT_COND_FORMAT, root ) ;
        return Evaluate_All_And_Get_Last_Node_Result( cond_root->right ) ;
      } // if

      condition = Evaluate( GetArgument( cond_root ) ) ;

      // If cond correct ( cond ( (1) 2 3 ) ) or ( cond ( '( 1 2 ) 3 4 ) )
      if ( condition->content == NULL || condition->content->type != NIL )
        return Evaluate_All_And_Get_Last_Node_Result( cond_root->right ) ;
      
      // Go to next condtion
      arg_cur = arg_cur->right ;
    } // while

    throw Exception( NO_RETURN_VALUE, root ) ;
    return CreateNode( "nil", NIL ) ;
  } // Eval_Cond()

  // return the last evaluated S-Expression or atom of a list
  Node* Evaluate_All_And_Get_Last_Node_Result( Node* statement_root ) {
    Node* cur = statement_root ;
    Node* result = Evaluate( cur->left ) ;

    while ( cur->right != NULL ) {
      cur = cur->right ;
      result = Evaluate( cur->left ) ;
    } // while
    
    return result ;
  } // Evaluate_All_And_Get_Last_Node_Result()


  // ---------- Functions ---------- //

  // cons    (2), list   (>=0)
  Node* Eval_Constructor( string function_name, Node* root ) {

    Node* arg_root = root->right ;

    // Go next node: first bypass argument
    Node* cur = arg_root ;

    // ---------- Function: cons ---------- //
    if ( function_name == "cons" ) {

      // check arguments count
      if ( ! mSemanticsAnalyzer_.Is_ArgCount_Equal_To( cur, 2 ) )
        throw Exception( INCORRECT_ARGUMENTS, function_name ) ;

      Node* first_arg = Evaluate( GetArgument( cur ) ) ;
      Node* second_arg = Evaluate( GetArgument( cur->right ) ) ;
      
      
      Node* result = SystemFunctions::CreateNode() ;
      // Paired Node
      result->left = first_arg ;

      // Check right node: ( cons 1 nil ) should be (1)
      if ( second_arg->content != NULL && second_arg->content->type == NIL ) 
        return result ;
      result->right = second_arg ;

      return result ;
      
    } // if

    // ---------- Function: list ---------- //
    else if ( function_name == "list" ) {
      
      // if args == 0
      if ( cur == NULL ) return CreateNode( "nil", NIL ) ;
      else if ( cur->content != NULL ) throw Exception( INCORRECT_LIST_FORMAT, root ) ;
      
      Node* result_root = SystemFunctions::CreateNode() ;
      Node* result_cur = result_root ;
      
      result_root->left = Evaluate( GetArgument( cur ) ) ;

      while ( cur->right != NULL ) {
        cur = cur->right ;
        // Pared Case
        if ( cur->content != NULL && cur->content->type != NIL )
          throw Exception( NON_LIST, root ) ;

        result_cur = SystemFunctions::Go_Next_Node_And_Create( result_cur ) ;
        result_cur->left = Evaluate( GetArgument( cur ) ) ;
      } // while

      return result_root ;

    } // else if

    return NULL ;
  } // Eval_Constructor()
  
  // '       (1), quote  (1)
  Node* Eval_Quote_Bypass( string function_name, Node* root ) {

    Node* arg_root = root->right ;

    if ( function_name == "quote" || function_name == "\'" ) {
      if ( ! mSemanticsAnalyzer_.Is_ArgCount_Equal_To( arg_root, 1 ) )
        throw Exception( INCORRECT_ARGUMENTS, function_name ) ;
      Node* first_arg = GetArgument( arg_root ) ;

      return first_arg ;
    } // if

    return NULL ;
  } // Eval_Quote_Bypass()

  // define  (2)
  // ( define ( SYMBOL zero-or-more-symbols ) one-or-more-S-expressions )
  // ( define SYMBOL S-expression )
  Node* Eval_Define( string function_name, Node* root ) {

    Node* arg_root = root->right ;

    // ---------- step1: check format ---------- //
    // if ( function_name != "define" ) throw Exception( TESTING, function_name ) ;
    
    // check arguments count and is no NULL
    if ( ! mSemanticsAnalyzer_.Is_ArgCount_BiggerEqual_Than( arg_root, 2 ) ) 
      throw Exception( INCORRECT_DEFINE_FORMAT, root ) ;

    // Paired tree constructure: ( define a . 5 ) case
    if ( arg_root->right != NULL && arg_root->right->content != NULL ) throw Exception( NON_LIST, root ) ;
    // ( define a 5 . 1 ) case, ( define a 5 . nil ) is ok
    else if ( arg_root->right->right != NULL &&
              arg_root->right->right->content != NULL &&
              arg_root->right->right->content->type != NIL ) throw Exception( NON_LIST, root ) ;

    Node* first_arg = GetArgument( arg_root ) ;
    Node* define_name = NULL ;
    Node* parameter = NULL ;
    Node* expression = NULL ;
    Node* binding = NULL ;

    // ---------- Define Function ---------- //
    if ( first_arg->content == NULL ) {
      define_name = GetArgument( first_arg ) ;
      if ( define_name == NULL || define_name->content == NULL || define_name->content->type != SYMBOL )
        throw Exception( INCORRECT_DEFINE_FORMAT, root ) ;

      // Have Parameter
      if ( define_name->right != NULL ) parameter = define_name->right ;
      else parameter = CreateNode( "nil", NIL ) ;
      expression = arg_root->right ;
      binding = CreateNode( CreateFunctionName( define_name->content->value ), SYMBOL ) ;
    } // if
    // ---------- Define Symbol ---------- //
    else {
      // check if define name has wrong format or naming 
      if ( first_arg->content->type != SYMBOL ) 
        throw Exception( INCORRECT_DEFINE_FORMAT, root ) ;
      else if ( TokenChecker::IsReserveWord( first_arg->content->value ) ) 
        throw Exception( INCORRECT_DEFINE_FORMAT, root ) ;
      
      define_name = first_arg ;
      binding = Evaluate( GetArgument( arg_root->right ) ) ;
    } // else


    // ---------- Insert to Symbol Table ---------- //
    
    
    gSymbolTable.Insert( define_name->content->value, parameter, expression, binding ) ;
    // cout << gSymbolTable.IsFunctionSymbol( define_name->content->value ) << endl ;
    cout << define_name->content->value << " defined\n" ;
    return NULL ;
  } // Eval_Define()

  // car     (1), cdr    (1)
  Node* Eval_Part_Accessors( string function_name, Node* root ) {

    Node* first_arg = root->right ; // access first bypass argument

    // Semantics Analyze argument count
    if ( ! mSemanticsAnalyzer_.Is_ArgCount_Equal_To( first_arg, 1 ) ) 
      throw Exception( INCORRECT_ARGUMENTS, function_name ) ;
    // check pared arguments count
    else if ( first_arg->right != NULL && first_arg->right->content != NULL ) 
      throw Exception( NON_LIST, root ) ;
    
    Node* result = Evaluate( GetArgument( first_arg ) ) ;
    
    // Not a List or Pared, can't part access
    if ( result->content != NULL ) 
      throw Exception( INCORRECT_ARGUMENT_TYPE, function_name, result->content ) ;

    // ---------- Function: car ---------- //
    if ( function_name == "car" ) {
      result = result->left ;
      if ( result == NULL ) return CreateNode( "nil", NIL ) ;
    } // if
    // ---------- Function: cdr ---------- //
    else if ( function_name == "cdr" ) {
      result = result->right ;
      if ( result == NULL ) return CreateNode( "nil", NIL ) ;
    } // else if
    
    return result ;
  } // Eval_Part_Accessors()

  // atom? pair? list? null? integer? real? number? string? boolean? symbol? (1)
  Node* Eval_Primitive_Predicate( string function_name, Node* root ) {
    Node* cur = root ;

    // Go next node: first bypass argument
    cur = cur->right ;
    
    // Semantics Analyze argument count
    if ( ! mSemanticsAnalyzer_.Is_ArgCount_Equal_To( cur, 1 ) )
      throw Exception( INCORRECT_ARGUMENTS, function_name ) ;

    Node* first_arg = Evaluate( GetArgument( cur ) ) ;

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
    else if ( function_name == "real?" || 
              function_name == "number?" ) {
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
  Node* Eval_Basic_Arithmetic( string function_name, Node* root ) {
    
    Node* result_root = SystemFunctions::CreateNode() ;
    Node* result_cur = result_root ;
    string op = function_name ;

    Node *byPassParameters = root->right ;
    Node* cur = byPassParameters ;

    // ---------- STEP1 Check Argument Count ---------- //
    if ( function_name == "not" ) {
      if ( ! mSemanticsAnalyzer_.Is_ArgCount_Equal_To( byPassParameters, 1 ) )
        throw Exception( INCORRECT_ARGUMENTS, op ) ;
    } // if
    else if ( ! mSemanticsAnalyzer_.Is_ArgCount_BiggerEqual_Than( byPassParameters, 2 ) ) {
      throw Exception( INCORRECT_ARGUMENTS, op ) ;
    } // else if
    
    // ---------- STEP2 Check Pared ---------- //
    // check the format: ( + "Hi" 2 ) int and float can't exist in basic operator with string
    // case1: ( + "Hi" 2 ) int and float can't exist in basic operator with string
    // case2: ( "string-append" "Hi" 2 ) int and float can't exist in basic operator with string
    // case3: ( / 2 0 ) divid zero
    // case4: ( / 2 0 . 2 ) paired list
    // case5: ( + '(1 2) '(3 4) ) had expanded, but still has a list 
    // mSemanticsAnalyzer_.ArithmeticCheck( function_name, result_root, root ) ;
    mSemanticsAnalyzer_.Check_No_Pared( root ) ;
    
    // ---------- STEP3 Evaluate ---------- //
    if ( function_name == "+" || function_name == "-" ||
         function_name == "*" || function_name == "/" )
      return BasicArithmaticCalculate( op, byPassParameters ) ;
    else if ( function_name == ">" || function_name == ">=" ||
              function_name == "<" || function_name == "<=" || function_name == "=" )
      return BasicComparing( op, byPassParameters ) ;
    else if ( function_name == "string-append" || function_name == "string>?" ||
              function_name == "string<?"      || function_name == "string=?" )
      return BasicString_Operate( op, byPassParameters ) ;
    else if ( function_name == "and" )
      return BasicAND( byPassParameters ) ;
    else if ( function_name == "or" )
      return BasicOR( byPassParameters ) ;
    else if ( function_name == "not" )
      return BasicNOT( byPassParameters ) ;
    
    return NULL ;
  } // Eval_Basic_Arithmetic()

  // eqv?    (2), equal? (2)
  Node* Eval_Equivalence( string function_name, Node* root ) {

    Node* cur = root ;
    // Go next node: first bypass argument
    cur = cur->right ;

    if ( ! mSemanticsAnalyzer_.Is_ArgCount_Equal_To( cur, 2 ) )
      throw Exception( INCORRECT_ARGUMENTS, function_name ) ;
    
    // ---------- Function: eqv? ---------- //
    if ( function_name == "eqv?" ) {
      return Eqv( cur ) ;
    } // if
    
    // ---------- Function: equal? ---------- //
    else if ( function_name == "equal?" ) {
      return Equal( cur ) ;
    } // else if
    
    return root ;
  } // Eval_Equivalence()

  // begin   (>=1)
  Node* Eval_Begin( string function_name, Node* root ) {
    Node* cur = root ;
    Node* arg_root = root->right ;

    if ( function_name == "begin" ) {
      if ( ! mSemanticsAnalyzer_.Is_ArgCount_BiggerEqual_Than( arg_root, 1 ) ) 
        throw Exception( INCORRECT_ARGUMENTS, function_name ) ;

      return Evaluate_All_And_Get_Last_Node_Result( arg_root ) ;
    } // if

    return root ;
  } // Eval_Begin()

  // if      (2 or 3), cond (>=1)
  Node* Eval_Condition( string function_name, Node* root ) {
    Node* cur = root ;
    Node* arg_root = root->right ;

    // ---------- Function: if ---------- //
    if ( function_name == "if" ) {
      // check argument count correct
      if ( ! mSemanticsAnalyzer_.Is_ArgCount_Equal_To( arg_root, 2, 3 ) )
        throw Exception( INCORRECT_ARGUMENTS, function_name ) ;

      return  Eval_If( cur ) ;
    } // if

    // ---------- Function: cond ---------- //
    else if ( function_name == "cond" ) {
      mSemanticsAnalyzer_.Check_Cond_Format( root ) ;
      return Eval_Cond( cur ) ;
    } // else if

    return root ;
  } // Eval_Condition()

  // clean-environment(0)
  Node* Eval_Clean_Environment( string function_name, Node* root ) {

    if ( ! mSemanticsAnalyzer_.Is_ArgCount_Equal_To( root->right, 0 ) )
      throw Exception( INCORRECT_ARGUMENTS, function_name ) ;
    gSymbolTable.Clear() ;
    
    cout << "environment cleaned\n" ;
    return NULL ;
  } // Eval_Clean_Environment()

  // exit(0)
  Node* Eval_Exit( string function_name, Node* root ) {

    if ( ! mSemanticsAnalyzer_.Is_ArgCount_Equal_To( root->right, 0 ) )
      throw Exception( INCORRECT_ARGUMENTS, function_name ) ;
    mIsQuit = true ;
    
    return NULL ;
  } // Eval_Exit()

  // ( let ( ... ) ......... )
  // '...' is a sequence of S-expressions: ( SYMBOL S-expression )
  // '.........' is a non-empty (!!!) sequence of S-expressions.
  // In words, 'let' has at least two parameters.
  Node* Eval_Let( string function_name, Node* root ) {

    // ---------- STEP1: Check Arguments Count ---------- //
    Node* arg_root = root->right ; // Get arguments
    if ( ! mSemanticsAnalyzer_.Is_ArgCount_BiggerEqual_Than( root->right, 2 ) )
      throw Exception( INCORRECT_LET_FORMAT ) ;

    Node* result = NULL ;
    
    // ---------- STEP2: Check Arguments Format ---------- //
    // Check Format: (( ... ) ( ... ) ) or ()
    mSemanticsAnalyzer_.Check_Let_Format_Correct( arg_root ) ;
    Node* func_parameter_root = GetArgument( arg_root ) ;
    
    // ---------- STEP3: Call Stack ---------- //
    try { 
      mCallStack_.Push() ; // No Name Function

      // there may be has a nil atom
      if ( func_parameter_root->content == NULL ) {
        FunctionSegment* calling_function = mCallStack_.Top() ;
        // Double Check there has a func seg
        if ( calling_function == NULL ) throw Exception( TESTING, "eval_let" ) ; // debug

        while ( func_parameter_root != NULL ) {
          
          Node* cur_arg = GetArgument( func_parameter_root ) ;
          Node* define_symbol = GetArgument( cur_arg ) ;
          Node* binding = Evaluate( GetArgument( cur_arg->right ) ) ;
          
          calling_function->ByPassParameter( define_symbol->content->value, binding ) ;
          func_parameter_root = func_parameter_root->right ;
        } // while

      } // if

      // ---------- STEP4: Eval Sequence of S-expression ---------- //
      Node* cur_arg = arg_root->right ;
      result = Evaluate_All_And_Get_Last_Node_Result( cur_arg ) ;

    } // try
    catch ( const Exception& e ) {
      mCallStack_.Pop() ; // if there is any exception, then callstack pop
      throw e ;
    } // catch

    mCallStack_.Pop() ;
    return result ;
  } // Eval_Let()

  // ( lambda ( zero-or-more-symbols ) one-or-more-S-expressions )
  Node* Eval_Lambda( string function_name, Node* root ) {
    
    // ---------- STEP1: Check Arguments Count ---------- //
    Node* arg_root = root->right ;
    if ( ! mSemanticsAnalyzer_.Is_ArgCount_BiggerEqual_Than( arg_root, 2 ) )
      throw Exception( INCORRECT_LAMBDA_FORMAT ) ;

    // ---------- STEP2: Check Arguments Format ---------- //
    mSemanticsAnalyzer_.Check_Lambda_Format_Correct( arg_root ) ;
    
    // ---------- STEP3: Record lambda function ---------- //
    Node* func_parameter_root = GetArgument( arg_root ) ;
    Node* expression = arg_root->right ;

    delete mLambda_Func_ ;
    mLambda_Func_ = new Symbol() ;
    mLambda_Func_->name = "#<procedure lambda>" ;
    mLambda_Func_->parameter = func_parameter_root ;
    mLambda_Func_->binding = expression ;

    return CreateNode( mLambda_Func_->name, SYMBOL ) ;
    // ---------- STEP3: Record lambda function ---------- //
    
    
    // gSymbolTable.Insert( "#<procedure lambda>", func_parameter_root, expression ) ;
    // return CreateNode( "lambda", SYMBOL ) ;

  } // Eval_Lambda()

  Node* Eval_Set( string function_name, Node* root ) {
    return root ;
  } // Eval_Set()

  Node* CallFunction( string function_name, Node* bypass_arg_root ) {

    Node* bypass_cur = SystemFunctions::CloneTree( bypass_arg_root ) ;
    Node* result = NULL ;

    // ---------- STEP1: Check Argument Count ---------- //
    Node* paramter = gSymbolTable.GetParameter( function_name ) ;
    mSemanticsAnalyzer_.Check_Call_Function_ByPass_Format( paramter, bypass_arg_root ) ;

    try {
      // ---------- STEP2: Call Stack ---------- //
      mCallStack_.Push( function_name, bypass_cur ) ; // It will automatically check format and bypass

      // ---------- STEP3: Eval Sequence of S-expression ---------- //
      FunctionSegment* calling_function = mCallStack_.Top() ;
      result = Evaluate_All_And_Get_Last_Node_Result( calling_function->GetExpression() ) ;
    } // try
    catch ( const Exception& e) {
      mCallStack_.Pop() ; // if there is any exception, then callstack pop
      throw e ;
    } // catch

    mCallStack_.Pop() ;
    return result ;
  } // CallFunction()

  // For lambda
  Node* CallFunction( Node* bypass_arg_root ) {
    

    // Get Function Information
    Node* funcPara_cur = SystemFunctions::CloneTree( mLambda_Func_->parameter ) ;
    Node* expression = SystemFunctions::CloneTree( mLambda_Func_->binding ) ;
    Node* result = NULL ;

    // ---------- STEP1: Check Argument Count ---------- //
    mSemanticsAnalyzer_.Check_Lambda_Call_Function_ByPass_Format( funcPara_cur, bypass_arg_root ) ;
    
    // Clone calling bypass
    Node* bypass_cur = SystemFunctions::CloneTree( bypass_arg_root ) ;

    try {
      // ---------- STEP2: Call Stack ---------- //
      mCallStack_.Push() ;
      FunctionSegment* calling_function = mCallStack_.Top() ;
      
      // ---------- STEP3: ByPass Parameter ---------- //
      if ( funcPara_cur != NULL && funcPara_cur->content == NULL ) {
        while ( funcPara_cur != NULL ) {
          Node* define_symbol = GetArgument( funcPara_cur ) ;

          // Bypass parameter
          calling_function->ByPassParameter( define_symbol->content->value, GetArgument( bypass_cur ) ) ;

          funcPara_cur = funcPara_cur->right ;
          bypass_cur = bypass_cur->right ; 
        } // while
      } // if

      // ---------- STEP4: Eval Sequence of S-expression ---------- //
      result = Evaluate_All_And_Get_Last_Node_Result( expression ) ;
    } // try
    catch ( const Exception& e) {
      mCallStack_.Pop() ; // if there is any exception, then callstack pop
      throw e ;
    } // catch

    mCallStack_.Pop() ;
    return result ;
  } // CallFunction()


  // ---------- Eval ---------- //
  Node* Evaluate( Node *root ) {
    
    // ---------- Initialize ---------- //
    mLevel_++ ;

    if ( root == NULL ) return NULL ;

    // ---------- CASE1: Atom || Symbol ---------- //
    // case1: Not a list
    // root node content has a value
    if ( root->content != NULL ) {

      // if what is being evaluated is an atom but not a symbol
      if ( TokenChecker::IsAtom( root->content->type ) && root->content->type != SYMBOL ) {
        // return that atom
        return root ;
      } // if

      // else if what is being evaluated is a symbol
      else if ( root->content->type == SYMBOL ) {
        // check whether it is bound to an S-expression or an internal function

        // Call Function Stack 
        if ( ( ! mCallStack_.IsEmpty() ) && mCallStack_.Top()->FindParameter( root->content->value ) )
              return mCallStack_.Top()->GetParameterBinding( root->content->value ) ;
        
        return gSymbolTable.GetBinding( root->content->value ) ; // mSymbolTable automatically check unbound
      } // else if
    } // if

    // ---------- CASE2: (...) ---------- //
    // what is being evaluated is (...)
    // root node content is NULL
    else {
      // ---------- Basic Check First Argument ---------- //
      Node *first_argument = root->left ;
      if ( root->left == NULL ) throw Exception( NON_LIST ) ;

      // First argument check: if (...) is not a (pure) list
      else if ( root->right != NULL && root->right->content != NULL &&
                root->right->content->type != NIL ) throw Exception( NON_LIST, root ) ;
      // First argument check: if is a list
      // else if ( root->left->content == NULL ) root->left = Evaluate( root->left ) ;
      
      // ---------- First Argument: HAS CONTENT ---------- //
      else if ( first_argument->content != NULL ) {

        // ---------- First Argument: NOT SYMBOL ---------- //
        if ( TokenChecker::IsAtom( first_argument->content->type ) &&
            first_argument->content->type != SYMBOL ) 
          throw Exception( NON_FUNCTION, first_argument->content ) ;
        
        // ---------- First Argument: SYMBOL ---------- //
        else if ( first_argument->content->type == SYMBOL || first_argument->content->type == QUOTE ) {
          
          bool IsUserDefineFunction = false ;
          // check if the SYM is the name of a known function

          // if ( first_argument->content->primitiveType == NONE ) {
            
          string request_symbol_name = first_argument->content->value ;
          // ---------- Check Call Stack ---------- //
          if ( ( ! mCallStack_.IsEmpty() ) && mCallStack_.Top()->FindParameter( request_symbol_name ) )
              first_argument = mCallStack_.Top()->GetParameterBinding( request_symbol_name ) ;
          else
            first_argument = gSymbolTable.GetBinding( request_symbol_name ) ;

          // ---------- Check Functional Name ---------- //
          // check if the SYM is the name of a known function
          CheckPrimitiveSymbol( first_argument->content ) ;

          if ( IsFunction( first_argument->content ) ) 
            IsUserDefineFunction = true ;
          else if ( first_argument->content->primitiveType == NONE )
            throw Exception( NON_FUNCTION, first_argument->content ) ;
          // } // if

          Token* functional_token = first_argument->content ;
          
          // if the current level is not the top level, and SYM is 'clean-environment'
          // or 'define' or　'exit'
          if ( functional_token->primitiveType == EXIT && mLevel_ != 1 ) {
            throw Exception( LEVEL_OF_EXIT ) ;
          } // if
          else if ( functional_token->primitiveType == DEFINE && mLevel_ != 1 ) {
            throw Exception( LEVEL_OF_DEFINE ) ;
          } // else if
          else if ( functional_token->primitiveType == CLEAN_ENVIRONMENT &&  mLevel_ != 1 ) {
            throw Exception( LEVEL_OF_CLEAN_ENVIRONMENT ) ;
          } // else if
            
          if ( IsUserDefineFunction ) {
            return CallFunction( SystemFunctions::GetFunctName( functional_token->value ), root->right ) ;
          } // if

          return Evaluate_Function( functional_token, root ) ;
          
        } // else if

        return NULL ;
      } // if

      // ---------- First Argument: HAS LIST ---------- //
      else { // the first argument of ( ... ) is ( 。。。 ), i.e., it is ( ( 。。。 ) ...... )

        bool IsFunction = false ;
        root->left = Evaluate( root->left ) ; // evaluate ( 。。。 )
        first_argument = root->left ;

        // check whether the evaluated result (of ( 。。。 )) is an internal function
        if ( first_argument->content != NULL ) {
          
          // check if the SYM is the name of a known function
          // Get Symbol Binding
          CheckPrimitiveSymbol( first_argument->content ) ;

          if ( first_argument->content->primitiveType == NONE ) {
              throw Exception( NON_FUNCTION, first_argument->content ) ;
          } // if

          Token* functional_token = first_argument->content ;

          // check if the symbol is user define function
          if ( functional_token->value == "#<procedure lambda>" && mLambda_Func_ != NULL ) {
            return CallFunction( root->right ) ; // (call function result)
          } // if

          // the evaluated result (of ( 。。。 )) is an internal function
          return Evaluate_Function( functional_token, root ) ;
        } // if
        else {
          throw Exception( NON_FUNCTION, first_argument ) ;
        } // else

      } // else
      
    } // else


    /*
      if what is being evaluated is an atom but not a symbol

        return that atom
        
      else if what is being evaluated is a symbol 

        check whether it is bound to an S-expression or an internal function

        if unbound
          ERROR (unbound symbol) : abc
        else 
          return that S-expression or internal function (i.e., its binding)

      else // what is being evaluated is (...) ; we call it the main S-expression below
           // this (...) cannot be nil (nil is an atom)

        if (...) is not a (pure) list
          ERROR (non-list) : (...)  // (...)要pretty print

        else if first argument of (...) is an atom ☆, which is not a symbol
          ERROR (attempt to apply non-function) : ☆

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

    return NULL ;
    
  } // Evaluate()

  Node* Evaluate_Function( Token* functional_token, Node* root ) {

    // ---------- LET ---------- //
    if ( functional_token->primitiveType == LET ) {
      return Eval_Let( SystemFunctions::GetFunctName( functional_token->value ), root ) ;
    } // else if

    // ---------- LAMBDA ---------- //
    else if ( functional_token->primitiveType == LAMBDA ) {
      // Binding #<procedure lambda> to a function
      return Eval_Lambda( SystemFunctions::GetFunctName( functional_token->value ), root ) ;
    } // else if

    // ---------- CONSTRUCTOR ---------- //
    else if ( functional_token->primitiveType == CONSTRUCTOR ) {
      return Eval_Constructor( SystemFunctions::GetFunctName( functional_token->value ), root ) ;
    } // else if

    // ---------- QUOTE_BYPASS ---------- //
    else if ( functional_token->primitiveType == QUOTE_BYPASS ) {
      return Eval_Quote_Bypass( SystemFunctions::GetFunctName( functional_token->value ), root ) ;
    } // else if

    // ---------- DEFINE ---------- //
    else if ( functional_token->primitiveType == DEFINE ) {
      return Eval_Define( SystemFunctions::GetFunctName( functional_token->value ), root ) ;

    } // else if

    // ---------- PART_ACCESSORS ---------- //
    else if ( functional_token->primitiveType == PART_ACCESSORS ) {
      return Eval_Part_Accessors( SystemFunctions::GetFunctName( functional_token->value ), root ) ;

    } // else if

    // ---------- PRIMITIVE_PREDICTATE ---------- //
    else if ( functional_token->primitiveType == PRIMITIVE_PREDICTATE ) {
      return Eval_Primitive_Predicate( SystemFunctions::GetFunctName( functional_token->value ), root ) ;

    } // else if

    // ---------- BASIC_ARITHMETIC ---------- //
    else if ( functional_token->primitiveType == BASIC_ARITHMETIC ) {
      return Eval_Basic_Arithmetic( SystemFunctions::GetFunctName( functional_token->value ), root ) ;

    } // else if

    // ---------- EQUIVALENCE ---------- //
    else if ( functional_token->primitiveType == EQUIVALENCE ) {
      return Eval_Equivalence( SystemFunctions::GetFunctName( functional_token->value ), root ) ;

    } // else if

    // ---------- BEGIN ---------- //
    else if ( functional_token->primitiveType == BEGIN ) {
      return Eval_Begin( SystemFunctions::GetFunctName( functional_token->value ), root ) ;
    } // else if

    // ---------- CONDITIONAL ---------- //
    else if ( functional_token->primitiveType == CONDITIONAL ) {
      return Eval_Condition( SystemFunctions::GetFunctName( functional_token->value ), root ) ;
    } // else if

    // ---------- CLEAN_ENVIRONMENT ---------- //
    else if ( functional_token->primitiveType == CLEAN_ENVIRONMENT ) {
      return Eval_Clean_Environment( SystemFunctions::GetFunctName( functional_token->value ), root ) ;
    } // else if

    // ---------- EXIT ---------- //
    else if ( functional_token->primitiveType == EXIT ) {
      return Eval_Exit( SystemFunctions::GetFunctName( functional_token->value ), root ) ;
    } // else if

    return NULL ;

  } // Evaluate_Function()

  bool IsFunction( Token* token ) {
    string name = token->value ;
    if ( name.size() < 13 ) return false ;
    if ( name.substr( 0, 12 ) == "#<procedure " && name.substr( name.size()-1, 1 ) == ">" )
      return true ;
    return false ;
  } // IsFunction()

  void CheckPrimitiveSymbol( Token* token ) {
    string name = token->value ;

    // <CONSTRUCTOR> := cons list 
    if ( name == "#<procedure cons>" || name == "#<procedure list>" )
      token->primitiveType = CONSTRUCTOR ;

    // <QUOTE_BYPASS> := ' quote
    else if ( name == "#<procedure \'>" || name == "#<procedure quote>" )
      token->primitiveType = QUOTE_BYPASS ;

    // <DEFINE> := define
    else if ( name == "#<procedure define>" )
      token->primitiveType = DEFINE ;

    // <PART_ACCESSORS> := car cdr
    else if ( name == "#<procedure car>" || name == "#<procedure cdr>" ) 
      token->primitiveType = PART_ACCESSORS ;

    // <PRIMITIVE_PREDICTATE> :=
    // atom? pair? list? null? integer? real? number? string? boolean? symbol?
    else if ( name == "#<procedure atom?>"    || name == "#<procedure pair?>"   ||
              name == "#<procedure list?>"    || name == "#<procedure null?>"   ||
              name == "#<procedure integer?>" || name == "#<procedure real?>"   ||
              name == "#<procedure number?>"  || name == "#<procedure string?>" ||
              name == "#<procedure boolean?>" || name == "#<procedure symbol?>" )
      token->primitiveType = PRIMITIVE_PREDICTATE ;
    
    // <BASIC_ARITHMETIC> :=
    // + - * / and or > >= < <= = not string>? string<? string=? string-append
    else if ( name == "#<procedure +>"    || name == "#<procedure ->"   ||
              name == "#<procedure *>"    || name == "#<procedure />"   ||
              name == "#<procedure and>"  || name == "#<procedure or>"  ||
              name == "#<procedure >>"    || name == "#<procedure >=>"  ||
              name == "#<procedure <>"    || name == "#<procedure <=>"  ||
              name == "#<procedure =>"    || name == "#<procedure not>" ||
              name == "#<procedure string>?>" || name == "#<procedure string<?>" ||
              name == "#<procedure string=?>" || name == "#<procedure string-append>" )
      token->primitiveType = BASIC_ARITHMETIC ;

    // <EQUIVALENCE> := eqv? equal?
    else if ( name == "#<procedure eqv?>" || name == "#<procedure equal?>" )
      token->primitiveType = EQUIVALENCE ;

    // <BEGIN> := begin
    else if ( name == "#<procedure begin>" )
      token->primitiveType = BEGIN ;

    // <CONDITIONAL> := if cond
    else if ( name == "#<procedure if>" || name == "#<procedure cond>"  )
      token->primitiveType = CONDITIONAL ;

    // <CLEAN_ENVIRONMENT> := clean-environment
    else if ( name == "#<procedure clean-environment>" )
      token->primitiveType = CLEAN_ENVIRONMENT ;

    // <EXIT> := exit
    else if ( name == "#<procedure exit>" )
      token->primitiveType = EXIT ;

    // <LET> := let
    else if ( name == "#<procedure let>" )
      token->primitiveType = LET ;

    // <LAMBDA> := lambda
    else if ( name == "#<procedure lambda>" )
      token->primitiveType = LAMBDA ;


  } // CheckPrimitiveSymbol()

  Token CopyToken( Node* node ) {
    Token newToken ;
    newToken.value = node->content->value ;
    newToken.type = node->content->type ;
    return newToken ;
  } // CopyToken()

  void Init() {
    if ( mLambda_Func_ != NULL ) delete mLambda_Func_ ;
    mLambda_Func_ = NULL ;
  } // Init()

public:

  Evaluator() {
    mCommand_ = new vector<Token>() ;
    mLambda_Func_ = NULL ;
    mLevel_ = 0 ;
    mIsQuit = false ;
  } // Evaluator()

  // Go through Right
  Node* Execute( Node *root ) {

    // set Top Level
    Init() ;
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

          ErrorHadling::ErrorMessage( e.mErrorType_, e.mToken_name_, e.mCurToken_, e.mCurNode_ ) ;
          gLineNum = 1 ;
          gColumn = 0 ; // while read the first char, col will add 1 automatically
        } // catch

      } // try
      catch ( const Exception& e ) {
        
        ErrorHadling::ErrorMessage( e.mErrorType_, e.mToken_name_, e.mCurToken_, e.mCurNode_ ) ;
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
  gAddress = 0 ;
  
  OurScheme ourSheme = OurScheme() ;
  ourSheme.Run() ;

  return 0 ;
} // main()
