#include "ast.h"
#include "st.h"

using namespace AST;

extern ST::SymbolTable symtab;

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
VAR::Variant_t Integer::computeTree(){
    return value;
}

VAR::Variant_t Double::computeTree(){
    return value;
}

VAR::Variant_t BinOp::computeTree(){
    VAR::Variant_t value, lvalue, rvalue;
    lvalue = left->computeTree();
    rvalue = right->computeTree();
    switch(op){
        case plus: value = lvalue + rvalue; break;
        case times: value = lvalue * rvalue; break;
        case assign:
            Variable* leftvar = dynamic_cast<Variable*>(left);
            symtab.entryList[leftvar->id].setValue(rvalue);
            value = symtab.entryList[leftvar->id].getValue();
    }
    return value;
}

VAR::Variant_t Block::computeTree(){
    VAR::Variant_t value;
    for (Node* line: lines) {
        value = line->computeTree();
        switch (value.getType()) {
          case VAR::integer_t: std::cout << "Computed " << value.getIntValue() << std::endl; break;
          case VAR::double_t: std::cout << "Computed " << value.getDoubleValue() << std::endl; break;
        }
    }
    return VAR::Variant_t(0);
}

VAR::Variant_t Variable::computeTree(){
    return symtab.entryList[id].getValue();
}
