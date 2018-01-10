import cppheaderparser
from pprint import pprint
import datetime
import re
import os

symbol_list = []
wrapper_name_counter = {}

out_file = None

def output(text):
    print(text)
    if out_file is not None:
        out_file.write(text)
        out_file.write("\n")

def get_method_path(method):
    if "path" in method:
        method_path = method["path"]
        if method_path.startswith("::"):
            method_path = method_path[2:]
        return method_path
    else:
        method_path = method["namespace"]
        if method_path.endswith("::"):
            method_path = method_path[:-2]
        return method_path

def get_method_wrapper_name(method):
    method_name = method["name"]
    if method["destructor"]:
        method_name = "destructor";
    name = "_" + get_method_path(method).replace("::", "_") + "_" + method_name
    if name in wrapper_name_counter:
        wrapper_name_counter[name] += 1
        name = name + str(wrapper_name_counter[name])
    else:
        wrapper_name_counter[name] = 1
    return name

def get_mangled_class_name(class_name):
    sp = class_name.split("::")
    ret = ""
    for p in sp:
        ret += str(len(p)) + p
    return ret

PRIMITIVE_TYPES = {
        "char": "c",
        "signed char": "c",
        "unsigned char": "h",
        "short": "s",
        "signed short": "s",
        "unsigned short": "t",
        "int": "i",
        "signed int": "i",
        "unsigned int": "j",
        "long": "l",
        "signed long": "l",
        "unsigned long": "m",
        "long long": "x",
        "unsigned long long": "y",
        "bool": "b",
        "float": "f",
        "double": "d",
}

def expand_cpp_default_templates(type_name):
    ret = re.sub(r"(std::unique_ptr\s*)<([\w:]*)>", r"\1<\2,std::default_delete<\2>>", type_name)
    ret = re.sub(r"(std::vector\s*)<([\w:]*)>", r"\1<\2,std::allocator<\2>>", ret)
    return ret

def get_mangled_type_name(type_name, substitutions):
    type_name = expand_cpp_default_templates(type_name)
    # print(type_name)

    sp = re.findall(r"((unsigned\s*|signed\s*|long\s*|short\s*|[\w:]+)+|[*&<>,])", type_name)
    ret = []
    last_type_start = []
    last_type_start.append(0)
    for p in sp:
        if type(p) is tuple:
            p = p[0]
        if p == "<":
            ret.append("I")
            last_type_start.append(len(ret))
        elif p == ">":
            ret.append("E")
            last_type_start.pop()
        elif p == ",":
            last_type_start[-1] = len(ret)
        elif p == "const":
            ret.insert(last_type_start[-1], "K")
        elif p == "&" and ret[last_type_start[-1]] == "R":
            ret[last_type_start[-1]] = "O"
        elif p == "&":
            ret.insert(last_type_start[-1], "R")
        elif p == "*":
            ret.insert(last_type_start[-1], "P")
        else:
            if p in PRIMITIVE_TYPES:
                ret.append(PRIMITIVE_TYPES[p])
                continue
            if p in substitutions:
                subId = substitutions.index(p)
                if subId > 0:
                    ret.append("S" + str(subId - 1) + "_")
                else:
                    ret.append("S_")
                continue
            np = p.split("::")
            if (np[0] == "std" or np[0] == "mcpe") and np[1] == "string":
                ret.append("Ss")
                continue
            if np[0] == "std" and np[1] == "allocator":
                ret.append("Sa")
                continue
            substitutions.append(p)
            f = True
            for pp in np:
                if f:
                    f = False
                    if pp == "std":
                        ret.append("St")
                        continue
                ret.append(str(len(pp)) + pp)
    return ''.join(ret)

def get_mangled_method(method):
    ret = "_ZN"
    if method["const"]:
        ret += "K"
    ret += get_mangled_type_name(get_method_path(method), [])
    if method["constructor"]:
        ret += "C2"
    elif method["destructor"]:
        ret += "D2"
    else:
        ret += str(len(method["name"])) + method["name"]
    ret += "E"
    if len(method["parameters"]) == 0:
        ret += "v"
    else:
        substitutions = []
        substitutions.append(get_method_path(method))
        for param in method["parameters"]:
            ret += get_mangled_type_name(param["type"], substitutions)
    return ret

def get_mangled_member(member, path):
    ret = "_ZN";
    ret += get_mangled_type_name(path, [])
    ret += str(len(member["name"])) + member["name"] + "E"
    return ret

def get_doxygen_properties(doxygen):
    properties = {}
    lines = doxygen.split('\n')
    for line in lines:
        if line.startswith("///"):
            line = line[3:]
        elif line.startswith("*"):
            line = line[1:]
        line = line.strip()
        if line.startswith("@"):
            key, _, val = line[1:].partition(" ")
            properties[key] = val

    return properties

def process_method(method, is_class):
    method_path = get_method_path(method)
    wrapper_name = get_method_wrapper_name(method)
    mangled_name = get_mangled_method(method)

    if "doxygen" in method:
        props = get_doxygen_properties(method["doxygen"])
        if "symbol" in props:
            mangled_name = props["symbol"]

    params_str = ""
    params_with_names = ""
    params_for_call = ""
    param_no = 1
    #if not method["static"]:
    #    params_str = method_path + "*"
    #    if method["const"]:
    #        params_str = method_path + " const*"
    #    params_for_call = "this"
    for param in method["parameters"]:
        if len(params_str) > 0:
            params_str += ", "
            params_for_call += ", "
        if len(params_with_names) > 0:
            params_with_names += ", "
        params_str += param["type"]
        params_with_names += param["type"] + " p" + str(param_no)
        if param["type"].startswith("std::unique_ptr"):
            params_for_call += "std::move(p" + str(param_no) + ")"
        else:
            params_for_call += "p" + str(param_no)
        param_no += 1
    ret_type = method["rtnType"]
    if ret_type.startswith("static "):
        ret_type = ret_type[len("static "):]
    if method["static"] or not is_class:
        output("static " + ret_type + " (*" + wrapper_name + ")(" + params_str + ");")
    else:
        output("static " + ret_type + " (" + method_path + "::*" + wrapper_name + ")(" + params_str + ")" + (" const" if method["const"] else "") + ";")
    output((ret_type + " " if not method["constructor"] and not method["destructor"] else "") + method_path + "::" + ("~" if method["destructor"] else "") + method["name"] + "(" + params_with_names + ")" + (" const" if method["const"] else "") + " {")
    has_return = ret_type != "void" and ret_type != ""
    if method["static"] or not is_class:
        output("    " + ("return " if has_return else "") + wrapper_name + "(" + params_for_call + ");")
    else:
        output("    " + ("return " if has_return else "") + "(this->*" + wrapper_name + ")(" + params_for_call + ");")
    output("}")
    symbol_list.append({
        "name": wrapper_name,
        "symbol": mangled_name
    })


def process_header(file):
    print("Processing file " + file)
    cpp_header = cppheaderparser.CppHeader(file)

    for function in cpp_header.functions:
        process_method(function, False)

    for class_name in cpp_header.classes:
        print("Processing class " + class_name)
        class_data = cpp_header.classes[class_name]
        # pprint(class_data)

        class_name_with_namespace = class_data["namespace"] + "::" + class_data["name"]
        if class_name_with_namespace.startswith("::"):
            class_name_with_namespace = class_name_with_namespace[2:]

        # pprint(class_data)
        for member_vis in class_data["properties"]:
            for member in class_data["properties"][member_vis]:
                if not member["static"]:
                    continue
                mangled_name = get_mangled_member(member, class_name_with_namespace)
                if "doxygen" in member:
                    props = get_doxygen_properties(member["doxygen"])
                    if "symbol" in props:
                        mangled_name = props["symbol"]
                m_type = member["type"]
                if m_type.startswith("static "):
                    m_type = m_type[len("static "):]
                output(m_type + " " + class_name_with_namespace + "::" + member["name"] + ";")
                symbol_list.append({
                    "name": class_name_with_namespace + "::" + member["name"],
                    "symbol": mangled_name
                })

        for method_vis in class_data["methods"]:
            for method in class_data["methods"][method_vis]:
                if method["defined"] or method["pure_virtual"]:
                    continue
                process_method(method, True)

                
def generate_init_func():
    output("void minecraft_symbols_init(void* handle) {")
    for symbol in symbol_list:
        output("    ((void*&) " + symbol["name"] + ") = hybris_dlsym(handle, \"" + symbol["symbol"] + "\");")
        output("    if (" + symbol["name"] + " == nullptr) Log::error(\"MinecraftSymbols\", \"Unresolved symbol: %s\", \"" + symbol["symbol"] + "\");")
    output("}")

out_file = open("../src/minecraft/symbols.cpp", "w")
output("// This file was automatically generated using tools/process_headers.py")
output("// Generated on " + datetime.datetime.utcnow().strftime("%a %b %d %Y %H:%M:%S UTC"))
output("")
output("#include <hybris/dlfcn.h>")
output("#include \"../log.h\"")
output("")
header_dir = "../src/minecraft/"
for file in os.listdir(header_dir):
    file_path = os.path.join(header_dir, file)
    if not os.path.isfile(file_path) or not file.endswith(".h"):
        continue
    if file == "symbols.h" or file == "string.h":
        continue
    output("#include \"" + file + "\"")
    process_header(file_path)
    output("")
generate_init_func()
out_file.close()

