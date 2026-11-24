#include "visualizer.hpp"
#include <fstream>

// ==========================================
// AsciiTreePrinter Implementation
// ==========================================

AsciiTreePrinter::AsciiTreePrinter(std::ostream& out) : m_out(out) {}

void AsciiTreePrinter::print(ASTNode& root) {
    m_indent = "";
    root.accept(*this);
}

void AsciiTreePrinter::printLine(const std::string& text, bool isLast) {
    m_out << m_indent << (isLast ? "└── " : "├── ") << text << "\n";
}

void AsciiTreePrinter::printChildren(const std::vector<ChildNode>& children) {
    std::string prevIndent = m_indent;
    for (size_t i = 0; i < children.size(); ++i) {
        bool last = (i + 1 == children.size());
        m_out << prevIndent << (last ? "└── " : "├── ");
        if (!children[i].tag.empty()) {
            m_out << children[i].tag << ": ";
        }
        if (children[i].node) {
            m_indent = prevIndent + (last ? "    " : "│   ");
            children[i].node->accept(*this);
        } else {
            m_out << "(null)\n";
        }
    }
    m_indent = prevIndent;
}

void AsciiTreePrinter::visit(ProgramNode& node) {
    m_out << "Program\n";
    std::vector<ChildNode> children;
    for (const auto& stmt : node.getStatements()) {
        children.push_back({"", stmt.get()});
    }
    printChildren(children);
}

void AsciiTreePrinter::visit(VarDeclNode& node) {
    m_out << "VarDecl (" << node.getType() << " " << node.getName() << ")\n";
    if (node.hasInit()) {
        std::vector<ChildNode> children = {{"Init", const_cast<ExprNode*>(node.getInitExpr())}};
        printChildren(children);
    }
}

void AsciiTreePrinter::visit(AssignNode& node) {
    m_out << "Assign (" << node.getName() << ")\n";
    std::vector<ChildNode> children = {{"Value", const_cast<ExprNode*>(node.getValue())}};
    printChildren(children);
}

void AsciiTreePrinter::visit(BlockNode& node) {
    m_out << "Block\n";
    std::vector<ChildNode> children;
    for (const auto& stmt : node.getStatements()) {
        children.push_back({"", stmt.get()});
    }
    printChildren(children);
}

void AsciiTreePrinter::visit(IfStmtNode& node) {
    m_out << "IfStmt\n";
    std::vector<ChildNode> children;
    children.push_back({"Condition", const_cast<ExprNode*>(node.getCondition())});
    children.push_back({"Then", const_cast<BlockNode*>(node.getThenBlock())});
    if (node.hasElse()) {
        children.push_back({"Else", const_cast<BlockNode*>(node.getElseBlock())});
    }
    printChildren(children);
}

void AsciiTreePrinter::visit(WhileStmtNode& node) {
    m_out << "WhileStmt\n";
    std::vector<ChildNode> children;
    children.push_back({"Condition", const_cast<ExprNode*>(node.getCondition())});
    children.push_back({"Body", const_cast<BlockNode*>(node.getBody())});
    printChildren(children);
}

void AsciiTreePrinter::visit(BinaryExprNode& node) {
    m_out << "BinaryExpr (" << node.getOp() << ")\n";
    std::vector<ChildNode> children;
    children.push_back({"Left", const_cast<ExprNode*>(node.getLeft())});
    children.push_back({"Right", const_cast<ExprNode*>(node.getRight())});
    printChildren(children);
}

void AsciiTreePrinter::visit(IdentifierNode& node) {
    m_out << "Identifier (" << node.getName() << ")\n";
}

void AsciiTreePrinter::visit(NumberLiteralNode& node) {
    m_out << "NumberLiteral (" << node.getValue() << ")\n";
}

// ==========================================
// DotExporter Implementation
// ==========================================

DotExporter::DotExporter(std::ostream& out) : m_out(out) {}

void DotExporter::exportDot(ASTNode& root) {
    m_nextId = 0;
    m_out << "digraph AST {\n";
    m_out << "  rankdir=TB;\n";
    m_out << "  node [fontname=\"Helvetica\", fontsize=11, style=\"rounded,filled\", margin=\"0.15,0.08\"];\n";
    m_out << "  edge [fontname=\"Helvetica\", fontsize=9];\n\n";
    root.accept(*this);
    m_out << "}\n";
}

bool DotExporter::exportToFile(ASTNode& root, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << filename << "' for DOT export\n";
        return false;
    }
    DotExporter exporter(file);
    exporter.exportDot(root);
    return true;
}

size_t DotExporter::createNode(const std::string& label, const std::string& shape,
                               const std::string& fillColor, const std::string& strokeColor) {
    size_t id = m_nextId++;
    m_out << "  node" << id << " [label=\"" << escapeDot(label)
          << "\", shape=" << shape
          << ", fillcolor=\"" << fillColor
          << "\", color=\"" << strokeColor << "\"];\n";
    m_currentId = id;
    return id;
}

void DotExporter::createEdge(size_t fromId, size_t toId, const std::string& label) {
    m_out << "  node" << fromId << " -> node" << toId;
    if (!label.empty()) {
        m_out << " [label=\"" << escapeDot(label) << "\"]";
    }
    m_out << ";\n";
}

std::string DotExporter::escapeDot(const std::string& str) {
    std::string out;
    for (size_t i = 0; i < str.size(); ++i) {
        char c = str[i];
        if (c == '"') {
            out += "\\\"";
        } else if (c == '\n') {
            out += "\\n";
        } else if (c == '\\') {
            if (i + 1 < str.size() && (str[i + 1] == 'n' || str[i + 1] == 'l' || str[i + 1] == 'r')) {
                out += '\\';
            } else {
                out += "\\\\";
            }
        } else {
            out += c;
        }
    }
    return out;
}

void DotExporter::visit(ProgramNode& node) {
    size_t myId = createNode("Program", "box", "#E1D5E7", "#9673A6");
    for (const auto& stmt : node.getStatements()) {
        if (stmt) {
            stmt->accept(*this);
            createEdge(myId, m_currentId);
        }
    }
    m_currentId = myId;
}

void DotExporter::visit(VarDeclNode& node) {
    std::string label = "VarDecl\n" + node.getType() + " " + node.getName();
    size_t myId = createNode(label, "box", "#DAE8FC", "#6C8EBF");
    if (node.hasInit() && node.getInitExpr()) {
        const_cast<ExprNode*>(node.getInitExpr())->accept(*this);
        createEdge(myId, m_currentId, "init");
    }
    m_currentId = myId;
}

void DotExporter::visit(AssignNode& node) {
    std::string label = "Assign\n" + node.getName() + " =";
    size_t myId = createNode(label, "box", "#DAE8FC", "#6C8EBF");
    if (node.getValue()) {
        const_cast<ExprNode*>(node.getValue())->accept(*this);
        createEdge(myId, m_currentId, "val");
    }
    m_currentId = myId;
}

void DotExporter::visit(BlockNode& node) {
    size_t myId = createNode("Block", "box", "#E1D5E7", "#9673A6");
    for (const auto& stmt : node.getStatements()) {
        if (stmt) {
            stmt->accept(*this);
            createEdge(myId, m_currentId);
        }
    }
    m_currentId = myId;
}

void DotExporter::visit(IfStmtNode& node) {
    size_t myId = createNode("IfStmt", "box", "#FFF2CC", "#D6B656");
    if (node.getCondition()) {
        const_cast<ExprNode*>(node.getCondition())->accept(*this);
        createEdge(myId, m_currentId, "cond");
    }
    if (node.getThenBlock()) {
        const_cast<BlockNode*>(node.getThenBlock())->accept(*this);
        createEdge(myId, m_currentId, "then");
    }
    if (node.hasElse() && node.getElseBlock()) {
        const_cast<BlockNode*>(node.getElseBlock())->accept(*this);
        createEdge(myId, m_currentId, "else");
    }
    m_currentId = myId;
}

void DotExporter::visit(WhileStmtNode& node) {
    size_t myId = createNode("WhileStmt", "box", "#FFF2CC", "#D6B656");
    if (node.getCondition()) {
        const_cast<ExprNode*>(node.getCondition())->accept(*this);
        createEdge(myId, m_currentId, "cond");
    }
    if (node.getBody()) {
        const_cast<BlockNode*>(node.getBody())->accept(*this);
        createEdge(myId, m_currentId, "body");
    }
    m_currentId = myId;
}

void DotExporter::visit(BinaryExprNode& node) {
    size_t myId = createNode("Op: " + node.getOp(), "diamond", "#FFE6CC", "#D79B00");
    if (node.getLeft()) {
        const_cast<ExprNode*>(node.getLeft())->accept(*this);
        createEdge(myId, m_currentId, "left");
    }
    if (node.getRight()) {
        const_cast<ExprNode*>(node.getRight())->accept(*this);
        createEdge(myId, m_currentId, "right");
    }
    m_currentId = myId;
}

void DotExporter::visit(IdentifierNode& node) {
    size_t myId = createNode("ID: " + node.getName(), "ellipse", "#D5E8D4", "#82B366");
    m_currentId = myId;
}

void DotExporter::visit(NumberLiteralNode& node) {
    size_t myId = createNode(std::to_string(node.getValue()), "ellipse", "#D5E8D4", "#82B366");
    m_currentId = myId;
}
