#include "ast.h"

using namespace AST;

extern ST::SymbolTable *symtab;

/* Print methods */
void Integer::printTree(){
    std::cout << value.getIntValue();
    return;
}

void Double::printTree(){
    std::cout << value.getDoubleValue();
    return;
}

void BinOp::printTree(){
    left->printTree();
    switch(op){
        case plus: std::cout << " + "; break;
        case times: std::cout << " * "; break;
        case assign: std::cout << " = "; break;
    }
    right->printTree();
    return;
}

void Block::printTree(){
    for (Node* line: lines) {
        line->printTree();
        std::cout << std::endl;
    }
}

void Variable::printTree(){
    if (next != NULL){
        next->printTree();
        std::cout << ", ";
    }
    std::cout << id;
}

/* Compute methods */
VAR::Variant_t Integer::computeTree(ST::SymbolTable *scope){
    return value;
}

VAR::Variant_t Double::computeTree(ST::SymbolTable *scope){
    return value;
}

VAR::Variant_t BinOp::computeTree(ST::SymbolTable *scope){
    VAR::Variant_t value, lvalue, rvalue;
    lvalue = left->computeTree(scope);
    rvalue = right->computeTree(scope);
    switch(op){
        case plus: value = lvalue + rvalue; break;
        case times: value = lvalue * rvalue; break;
        case assign:
            Variable* leftvar = dynamic_cast<Variable*>(left);
            scope->getSymbol(leftvar->id).setValue(rvalue);
            value = scope->getSymbol(leftvar->id).getValue();
            break;
    }
    return value;
}

VAR::Variant_t Block::computeTree(ST::SymbolTable *scope){
    VAR::Variant_t value;
    for (Node* line: lines) {
        value = line->computeTree(this->scope);
        switch (value.getType()) {
          case VAR::integer_t: std::cout << "Computed " << value.getIntValue() << std::endl; break;
          case VAR::double_t: std::cout << "Computed " << value.getDoubleValue() << std::endl; break;
        }
    }
    return VAR::Variant_t();
}

VAR::Variant_t Variable::computeTree(ST::SymbolTable *scope){
    return scope->getSymbol(id).getValue();
}
