#include "MiniTriton/AST/ASTPrinter.h"
#include <sstream>

namespace minitriton {

void ASTPrinter::indent() {
    for (int i = 0; i < indentLevel_; i++) {
        result_ += "  ";
    }
}

void ASTPrinter::writeLine(const std::string& text) {
    indent();
    result_ += text + "\n";
}

void ASTPrinter::write(const std::string& text) {
    result_ += text;
}

std::string ASTPrinter::print(Module& module) {
    result_.clear();
    indentLevel_ = 0;
    module.accept(*this);
    return result_;
}

void ASTPrinter::visit(Module& node) {
    writeLine("Module");
    indentLevel_++;
    for (auto& fn : node.functions) {
        fn->accept(*this);
    }
    indentLevel_--;
}

void ASTPrinter::visit(FunctionDef& node) {
    std::string header = (node.isKernel ? "@kernel " : "") +
                         std::string("def ") + node.name + "(";
    for (size_t i = 0; i < node.params.size(); i++) {
        if (i > 0) header += ", ";
        header += node.params[i].name + ": " + node.params[i].type->toString();
    }
    header += ")";
    writeLine(header);
    indentLevel_++;
    for (auto& stmt : node.body) {
        stmt->accept(*this);
    }
    indentLevel_--;
}

void ASTPrinter::visit(AssignStmt& node) {
    indent();
    write(node.target + " = ");
    node.value->accept(*this);
    write("\n");
}

void ASTPrinter::visit(ExprStmt& node) {
    indent();
    node.expr->accept(*this);
    write("\n");
}

void ASTPrinter::visit(ReturnStmt& node) {
    indent();
    write("return");
    if (node.value) {
        write(" ");
        node.value->accept(*this);
    }
    write("\n");
}

void ASTPrinter::visit(NumberLiteral& node) {
    write(node.raw);
}

void ASTPrinter::visit(IdentifierExpr& node) {
    write(node.name);
}

void ASTPrinter::visit(BinaryExpr& node) {
    write("(");
    node.left->accept(*this);
    write(" " + std::string(BinaryExpr::opToString(node.op)) + " ");
    node.right->accept(*this);
    write(")");
}

void ASTPrinter::visit(UnaryExpr& node) {
    write("(-");
    node.operand->accept(*this);
    write(")");
}

void ASTPrinter::visit(CallExpr& node) {
    write(node.callee + "(");
    for (size_t i = 0; i < node.args.size(); i++) {
        if (i > 0) write(", ");
        node.args[i]->accept(*this);
    }
    write(")");
}

} // namespace minitriton
