#include "st.hpp"
#include "ast.hpp"

using namespace ST;

bool SymbolTable::checkId(std::string id, bool creation/*=false*/){
  bool result = _entryList.find(id) != _entryList.end();
  // Caso seja a declaração de uma variavel
  // só é preciso checar o primeiro nivel
  // do escopo
  if(creation)
    return result;
  return result ? result :
    (_previous!=nullptr ? _previous->checkId(id, creation) : false);
}

void SymbolTable::addSymbol(std::string id, Symbol *newsymbol){
  _entryList[id] = newsymbol;
}

/* Procura o id passado em todos os escopos e
    retorno o simbolo deste id. */
Symbol* SymbolTable::getSymbol(std::string id){
  bool result = _entryList.find(id) != _entryList.end();
  if(result)
    return _entryList[id];
  else if(_previous != nullptr)
    return _previous->getSymbol(id);
  else
    return nullptr;
}

AST::Node* SymbolTable::newVariable(std::string id, AST::Node* next, bool isArray){
  Kinds::Kind kind = isArray ? Kinds::array_t : Kinds::variable_t;
  if ( checkId(id, true) ){
    auto symbol = getSymbol(id);
    Errors::print(Errors::redefinition,
      Kinds::kindName[symbol->kind], id.c_str());
  }else {
     Symbol *entry = new Symbol(kind);
     addSymbol(id,entry); //Adds variable to symbol table
  }
  if(isArray)
    return new AST::Array(id, next, AST::declr);
  else
    return new AST::Variable(id, next, AST::declr);
}

AST::Node* SymbolTable::declFunction(std::string id, AST::Node *params, Types::Type type){
  if ( checkId(id, true) ){
    Errors::print(Errors::func_redeclaration, id.c_str());
  }else {
    Symbol *entry = new Symbol(params, type);
    addSymbol(id, entry);
  }
  return new AST::Function(id, params, nullptr, AST::declr, type);
}

AST::Node* SymbolTable::defFunction(std::string id, AST::Node *params,
  AST::Node *block, Types::Type type){

  auto params1 = (AST::Variable*)params;
  auto symbol = getSymbol(id);
  if(symbol != nullptr){
    auto params2 = (AST::Variable*)symbol->params;
    bool testKind = symbol->kind == Kinds::function_t;
    bool testType = symbol->type == type;
    bool testParams = (params2!=nullptr?params2->equals(params1, true):params1==nullptr);

    // Checa redefinição da função, seja por ter alguma parte
    // diferente ou por ter tudo igual a alguma outra função
    // ja declarada/definida
    if( (!testKind || !testType || !testParams) ||
        (testKind && testType && testParams && symbol->initialized)){
      Errors:print(Errors::redefinition, Kinds::kindName[symbol->kind],
        id.c_str());
      type = Types::unknown_t;
    }else{
      symbol->initialized = true;
    }
  }else{
    Symbol *entry = new Symbol(params, type);
    entry->initialized = true;
    addSymbol(id, entry);
  }

  return new AST::Function(id, params, block, AST::def, type);
}

AST::Node* SymbolTable::assignVariable(std::string id){
  if ( ! checkId(id) ) Errors::print(Errors::without_declaration,
    Kinds::kindName[Kinds::variable_t], id.c_str());

  auto symbol = getSymbol(id);
  auto type = Types::unknown_t;
  if(symbol != nullptr){
    if(symbol->kind != Kinds::variable_t){
      Errors::print(Errors::wrong_use, Kinds::kindName[symbol->kind], id.c_str(),
        Kinds::kindName[Kinds::variable_t]);
    }else{
      symbol->initialized = true;
      type = symbol->type;
    }
  }
  return new AST::Variable(id, NULL, AST::attr, type);
}

AST::Node* SymbolTable::assignArray(std::string id, AST::Node *index){
  if( !checkId(id) ) Errors::print(Errors::without_declaration,
    Kinds::kindName[Kinds::function_t], id.c_str());

  auto symbol = getSymbol(id);
  auto type = Types::unknown_t;
  if(symbol != nullptr){
    if(symbol->kind != Kinds::array_t)
      Errors::print(Errors::wrong_use, Kinds::kindName[symbol->kind],
        id.c_str(), Kinds::kindName[Kinds::array_t]);
    else
      type = symbol->type;
  }
  if(index == nullptr || index->type != Types::integer_t)
    Errors::print(Errors::index_wrong_type, Types::mascType[index->type]);

  return new AST::Array(id, index, AST::attr, type);
}

/* 'useOfFunc' significa que a variavel esta sendo usada
     no uso de uma função. Ex: c := soma(a, b);
    Isto é uma gambiarra para que possa ser usado arranjos
     como parametros de funções                             */
AST::Node* SymbolTable::useVariable(std::string id, bool useOfFunc){
  if ( ! checkId(id) ) Errors::print(Errors::without_declaration,
    Kinds::kindName[Kinds::variable_t], id.c_str());

  auto symbol = getSymbol(id);
  auto type = Types::unknown_t;
  if ( symbol != nullptr ){
    type = symbol->type;
    if(!useOfFunc && symbol->kind != Kinds::variable_t){
      Errors::print(Errors::wrong_use, Kinds::kindName[symbol->kind],
        id.c_str(), Kinds::kindName[Kinds::variable_t]);
      type = Types::unknown_t;
    }else if(!useOfFunc && checkId(id,true) && !symbol->initialized){
      Errors::print(Errors::not_initialized, Kinds::kindName[Kinds::variable_t],
        id.c_str());
    }
  }
  if(useOfFunc && symbol != nullptr && symbol->kind == Kinds::array_t)
    return new AST::Array(id,nullptr,AST::read,symbol->aSize,type);
  else
    return new AST::Variable(id, NULL, AST::read, type);
}

AST::Node* SymbolTable::useArray(std::string id, AST::Node *index){
  if( !checkId(id) ) Errors::print(Errors::without_declaration,
    Kinds::kindName[Kinds::array_t], id.c_str());

  auto symbol = getSymbol(id);
  auto type = Types::unknown_t;
  int size = 0;
  if(symbol != nullptr){
    if(symbol->kind != Kinds::array_t)
      Errors::print(Errors::wrong_use, Kinds::kindName[symbol->kind],
        id.c_str(), Kinds::kindName[Kinds::array_t]);
    else{
      type = symbol->type;
      size = symbol->aSize;
    }
  }
  if(index == nullptr || index->type != Types::integer_t)
    Errors::print(Errors::index_wrong_type, Types::mascType[index->type]);

  return new AST::Array(id, nullptr, index, AST::read, size, type);
}

AST::Node* SymbolTable::useFunc(std::string id, AST::Node *params){
  if(!checkId(id)) Errors::print(Errors::without_declaration,
      Kinds::kindName[Kinds::function_t], id.c_str());

  //num declared params, num used params
  int nd = 0, nu = 0;
  auto symbol = getSymbol(id);
  auto type = Types::unknown_t;
  auto uparams = params;
  bool defined = true;
  if(symbol != nullptr){
    // uso errado [18]
    if(symbol->kind != Kinds::function_t)
      Errors::print(Errors::wrong_use, Kinds::kindName[symbol->kind],
        id.c_str(), Kinds::kindName[Kinds::function_t]);
    else
      type = symbol->type;

    auto dparams = symbol->params;
    while(uparams != nullptr){
      if(dparams != nullptr){
        // param de tipo imcompativel [15]
        if(uparams->type != dparams->type)
          Errors::print(Errors::param_wrong_type,Types::mascType[dparams->type],
            Types::mascType[uparams->type]);
        if(dparams->getNodeType()  == AST::array_nt){
          auto uparams2 = uparams;
          // Gambiarra para resolver coisas como: arranjador((v));
          if(uparams->getNodeType() == AST::uniop_nt){
            auto tmp = dynamic_cast<AST::UniOp*>(uparams);
            while(tmp!=nullptr && tmp->op == Ops::u_paren){
              if(tmp->right->getNodeType() == AST::uniop_nt)
                tmp = dynamic_cast<AST::UniOp*>(tmp->right);
              else{
                uparams2 = tmp->right;
                tmp = nullptr;
              }
            }
          }
          if(uparams2->getNodeType() != AST::array_nt)
            Errors::print(Errors::param_value_as_array);
          else{
            // tamanho do arranjo tem q ser maior ou igual ao do param [16]
            auto ar = dynamic_cast<AST::Array*>(uparams2);
            if(ar->size < dynamic_cast<AST::Array*>(dparams)->size)
              Errors::print(Errors::array_size_lst_needed, ar->id.c_str());
          }
        }
        ++nd; dparams = dparams->next;
      }
      ++nu; uparams = uparams->next;
    }
    while(dparams != nullptr){
      ++nd; dparams = dparams->next;
    }
  }else{
    while(uparams != nullptr){
      ++nu; uparams = uparams->next;
    }
    defined = false;
  }

  // numero de parametros [23]
  if(nd != nu)
    Errors::print(Errors::func_wrong_param_amount,
      id.c_str(), nd, nu);

  // função não declarada deve retornar nó com 0 parametros [25]
  return new AST::Function(id, (defined?params:nullptr), nullptr, AST::read, type);
}

/* Seta o tipo de toda a sequencia de variaveis.
   Ex: int: a, b, c; (tipo é setado depois de criado os Nodos) */
void SymbolTable::setType(AST::Node *node, Types::Type type){
  AST::Variable *tmp = (AST::Variable*) node;
  Symbol *tmp2 = nullptr;
  while(tmp != nullptr){
    tmp2 = getSymbol(tmp->id);
    if(tmp2->type == Types::unknown_t){
      tmp->setType(type);
      tmp2->type = type;
    }
    tmp = (AST::Variable*) tmp->next;
  }
}

void SymbolTable::setArraySize(AST::Node *node, int aSize){
  AST::Array *tmp = (AST::Array*) node;
  while(tmp != nullptr){
    tmp->setSize(aSize);
    getSymbol(tmp->id)->aSize = aSize;
    tmp = (AST::Array*) tmp->next;
  }
}

/* Adiciona os parametros da declaração ou definição
    de uma função ao escopo do corpo dela             */
void SymbolTable::addFuncParams(AST::Node *oldParams, AST::Node *newParams){
  AST::Variable *var = nullptr;
  if(oldParams == nullptr)// Função ja foi declarada
    var = (AST::Variable*) newParams;
  else
    var = (AST::Variable*) oldParams;

  while(var != nullptr){
    Symbol *entry = new Symbol(var->getKind());
    entry->type = var->type;
    entry->initialized = true;
    if(var->getNodeType() == AST::array_nt)
      entry->aSize = dynamic_cast<AST::Array*>(var)->size;

    addSymbol(var->id,entry);

    var = (AST::Variable*) var->next;
  }
}

/* Verifica se todas as funções declaradas
    foram definidas                         */
void SymbolTable::checkFuncs(){
  for(const auto& iter : _entryList){
    const auto& symbol = iter.second;
    if(symbol->kind == Kinds::function_t && !symbol->initialized)
      Errors::print(Errors::func_never_declared, iter.first.c_str());
  }
}
