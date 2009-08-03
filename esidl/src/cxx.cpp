/*
 * Copyright 2008, 2009 Google Inc.
 * Copyright 2007 Nintendo Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cplusplus.h"
#include "info.h"

class Cxx : public CPlusPlus
{
    void visitInterfaceElement(const Interface* interface, Node* element)
    {
        if (element->isSequence(interface) || element->isNative(interface))
        {
            return;
        }
        optionalStage = 0;
        do
        {
            optionalCount = 0;
            element->accept(this);
            ++optionalStage;
        } while (optionalStage <= optionalCount);
    }

public:
    Cxx(const char* source, FILE* file, const char* stringTypeName = "char*", bool useExceptions = true) :
        CPlusPlus(source, file, stringTypeName, useExceptions)
    {
    }

    virtual void at(const ExceptDcl* node)
    {
        writetab();
        if (node->getJavadoc().size())
        {
            write("%s\n", node->getJavadoc().c_str());
            writetab();
        }
        write("struct %s {\n", node->getName().c_str());
            printChildren(node);
        writeln("};");
    }

    virtual void at(const Interface* node)
    {
        if (node->getAttr() & Interface::ImplementedOn)
        {
            return;
        }
        writetab();
        if (node->getJavadoc().size())
        {
            write("%s\n", node->getJavadoc().c_str());
            writetab();
        }
        if (node->isLeaf())
        {
            write("class %s;\n", node->getName().c_str());
        }
        else
        {
            write("class %s", node->getName().c_str());
            if (node->getExtends())
            {
                write(" : ");
                for (NodeList::iterator i = node->getExtends()->begin();
                     i != node->getExtends()->end();
                     ++i)
                {
                    if (i != node->getExtends()->begin())
                    {
                        write(", ");
                    }
                    write("public ");
                    (*i)->accept(this);
                }
            }
            write(" {\n");
            unindent();
            writeln("public:");
            indent();

            for (NodeList::iterator i = node->begin(); i != node->end(); ++i)
            {
                visitInterfaceElement(node, *i);
            }

            // Expand mixins
            for (std::list<const Interface*>::const_iterator i = node->getMixins()->begin();
                 i != node->getMixins()->end();
                 ++i)
            {
                writeln("// %s", (*i)->getName().c_str());
                for (NodeList::iterator j = (*i)->begin(); j != (*i)->end(); ++j)
                {
                    visitInterfaceElement(*i, *j);
                }
            }

            writeln("static const char* iid() {");
                writeln("static const char* const name = \"%s\";",
                        node->getQualifiedName().c_str());
                writeln("return name;");
            writeln("}");

            writeln("static const char* info() {");
                writetab();
                write("static const char* const info =");
                flush();
                indent();
                Info info(this, moduleName);
                const_cast<Interface*>(node)->accept(&info);
                info.flush();
                write(";\n");
                unindent();
                writeln("return info;");
            writeln("}");

            if (Interface* constructor = node->getConstructor())
            {
                // Process constructors.
                constructorMode = true;
                for (NodeList::iterator i = constructor->begin();
                     i != constructor->end();
                     ++i)
                {
                    (*i)->accept(this);
                }
                constructorMode = false;
                writeln("static Constructor* getConstructor() {");
                    writeln("return constructor;");
                writeln("}");
                writeln("static void setConstructor(Constructor* ctor) {");
                    writeln("constructor = ctor;");
                writeln("}");
                unindent();
                writeln("private:");
                indent();
                writeln("static Constructor* constructor;");
            }

            writeln("};");

            if (node->getConstructor())
            {
                // TODO: Control the use of GCC extensions.
                writeln("%s::Constructor* %s::constructor __attribute__((weak));",
                        node->getName().c_str(), node->getName().c_str());
            }
        }
    }

    virtual void at(const BinaryExpr* node)
    {
        node->getLeft()->accept(this);
        write(" %s ", node->getName().c_str());
        node->getRight()->accept(this);
    }

    virtual void at(const UnaryExpr* node)
    {
        write("%s", node->getName().c_str());
        NodeList::iterator elem = node->begin();
        (*elem)->accept(this);
    }

    virtual void at(const GroupingExpression* node)
    {
        write("(");
        NodeList::iterator elem = node->begin();
        (*elem)->accept(this);
        write(")");
    }

    virtual void at(const Literal* node)
    {
        write("%s", node->getName().c_str());
    }

    virtual void at(const Member* node)
    {
        writetab();
        if (node->isTypedef(node->getParent()))
        {
            write("typedef ");
        }
        node->getSpec()->accept(this);
        write(" %s;\n", node->getName().c_str());
    }

    virtual void at(const ArrayDcl* node)
    {
        assert(!node->isLeaf());
        writetab();
        if (node->isTypedef(node->getParent()))
        {
            write("typedef ");
        }
        node->getSpec()->accept(this);
        write(" %s", node->getName().c_str());
        for (NodeList::iterator i = node->begin(); i != node->end(); ++i)
        {
            write("[");
            (*i)->accept(this);
            write("]");
        }
        if (node->isTypedef(node->getParent()))
        {
            write(";\n");
        }
    }

    virtual void at(const ConstDcl* node)
    {
        writetab();
        if (node->getJavadoc().size())
        {
            write("%s\n", node->getJavadoc().c_str());
            writetab();
        }
        write("static const ");
        node->getSpec()->accept(this);
        write(" %s = ", node->getName().c_str());
        node->getExp()->accept(this);
        write(";\n");
    }

    virtual void at(const Attribute* node)
    {
        writetab();
        if (node->getJavadoc().size())
        {
            write("%s\n", node->getJavadoc().c_str());
            writetab();
        }

        // getter
        CPlusPlus::getter(node);
        write(" = 0;\n");

        if (!node->isReadonly() || node->isPutForwards() || node->isReplaceable())
        {
            // setter
            writetab();
            CPlusPlus::setter(node);
            write(" = 0;\n");
        }
    }

    virtual void at(const OpDcl* node)
    {
        writetab();
        if (node->getJavadoc().size())
        {
            write("%s\n", node->getJavadoc().c_str());
            writetab();
        }

        CPlusPlus::at(node);

        if (!constructorMode)
        {
            write(" = 0;\n");
        }
        else
        {
            write("{");
            writeln("");
                writeln("if (constructor)");
                indent();
                    writetab();
                    write("return constructor->createInstance(");
                    for (NodeList::iterator i = node->begin(); i != node->end(); ++i)
                    {
                        if (i != node->begin())
                        {
                            write(", ");
                        }
                        write("%s", (*i)->getName().c_str());
                    }
                    write(");");
                    writeln("");
                unindent();
                writeln("else");
                indent();
                    writeln("return 0;");
                unindent();
            writeln("}");
        }
    }
};

class Import : public Visitor
{
    const char* source;
    FILE* file;
    bool printed;

public:
    Import(const char* source, FILE* file) :
        source(source),
        file(file),
        printed(false)
    {
    }

    virtual void at(const Node* node)
    {
        visitChildren(node);
    }

    virtual void at(const Include* node)
    {
        if (!node->isDefinedIn(source))
        {
            return;
        }
        if (node->isSystem())
        {
            fprintf(file, "#include <%s>\n", getOutputFilename(node->getName().c_str(), "h").c_str());
        }
        else
        {
            fprintf(file, "#include \"%s\"\n", getOutputFilename(node->getName().c_str(), "h").c_str());
        }
        printed = true;
    }

    bool hasPrinted() const
    {
        return printed;
    }
};

class Predeclaration : public Visitor, public Formatter
{
    const char* source;

public:
    Predeclaration(const char* source, FILE* file) :
        Formatter(file),
        source(source)
    {
    }

    virtual void at(const Node* node)
    {
        if (!node->isDefinedIn(source))
        {
            return;
        }
        visitChildren(node);
    }

    virtual void at(const Module* node)
    {
        if (!node->hasPredeclarations())
        {
            return;
        }
        if (0 < node->getName().size())
        {
            write("namespace %s {\n", node->getName().c_str());
                visitChildren(node);
            writeln("}");
            writeln("");
        }
        else
        {
            visitChildren(node);
        }
    }

    virtual void at(const Interface* node)
    {
        if (!node->isDefinedIn(source) || !node->isLeaf())
        {
            return;
        }
        writeln("class %s;", node->getName().c_str());
    }
};

void printCxx(const char* source, const char* stringTypeName, bool useExceptions)
{
    const std::string filename = getOutputFilename(source, "h");
    printf("# %s\n", filename.c_str());

    FILE* file = fopen(filename.c_str(), "w");
    if (!file)
    {
        return;
    }

    std::string included = getIncludedName(filename);
    fprintf(file, "// Generated by esidl %s.\n\n", VERSION);
    fprintf(file, "#ifndef %s\n", included.c_str());
    fprintf(file, "#define %s\n\n", included.c_str());

    Import import(source, file);
    getSpecification()->accept(&import);
    if (import.hasPrinted())
    {
        fprintf(file, "\n");
    }

    if (!Node::getFlatNamespace())
    {
        Predeclaration predeclaration(source, file);
        getSpecification()->accept(&predeclaration);
    }

    Cxx cxx(source, file, stringTypeName, useExceptions);
    getSpecification()->accept(&cxx);

    fprintf(file, "#endif  // %s\n", included.c_str());

    fclose(file);
}
