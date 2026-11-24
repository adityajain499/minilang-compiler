#include "ast.hpp"

void ProgramNode::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void VarDeclNode::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void AssignNode::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void BlockNode::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void IfStmtNode::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void WhileStmtNode::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void BinaryExprNode::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void IdentifierNode::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}

void NumberLiteralNode::accept(ASTVisitor& visitor) {
    visitor.visit(*this);
}
