#include <xdev/template-blocks.hpp>
#include <xdev/tools.hpp>

#include <iostream>

using namespace xdev;
using namespace xdev::temp;

//
// BaseBlock
//

BaseBlock::BaseBlock(const XResources::ptr & resources) :
    m_resources(resources)
{

}
BaseBlock::~BaseBlock() {}

bool BaseBlock::ExtractFunction(const string& raw_content, string& function_name, vector<string>& args)
{
    static const regex function_expr(R"(\s*([a-zA-Z_]+)\((.*)\)\s*$)");
    cmatch what;
    if (!regex_match(raw_content.c_str(), what, function_expr))
    {
        return false;
    }
    function_name = what[1];
    tools::trim(function_name);
    vector<string> raw_args = tools::split(what[2], ',', true);
    tools::fast_foreach(raw_args, [&args](string& arg)
    {
        tools::trim(arg);
        args.push_back(arg);
    });
    return true;
}

//
// RenderBlock
//

RenderBlock::RenderBlock(const string& content, const XResources::ptr& resources):
    BaseBlock(resources),
    m_expression(Expression::Load(content))
{
}

string RenderBlock::process(const xdict& context)
{
    try {
        // std::cout << XVariant(context).toJson() << std::endl << std::endl;
        return m_expression->eval(context, m_resources).toString();
    }
    catch (const exception& err) {
        throw Error("Evaluating: \"" + m_expression->content() + "\"", err);
    }
}

string RenderBlock::BlockStart = "{{";
string RenderBlock::BlockEnd = "}}";

RenderBlock::ptr RenderBlock::TryLoad(const string& input, size_t& offset, const XResources::ptr& resources)
{
    if (input.compare(offset, BlockStart.size(), BlockStart) == 0)
    {
        size_t start = offset + BlockStart.size();
        size_t end = input.find(BlockEnd, start);
        if (end == string::npos)
        {
            return nullptr;
        }
        offset = end + BlockEnd.size();
        string block = input.substr(start, end - start);
        tools::trim(block);
        return make_shared<RenderBlock>(block, resources);
    }
    else
    {
        return nullptr;
    }
}

//
// ControlBlock
//

string ControlBlock::BlockStart = "{%";
string ControlBlock::BlockEnd = "%}";

ssize_t ControlBlock::NextBlockOf(const string& input, const vector<string>& commands, size_t offset, block_match_t& match)
{
    match.block_end.end = offset;
    while ((match.block_start.begin = input.find(BlockStart, match.block_end.end)) != string::npos)
    {
        match.block_start.end = match.block_start.begin + BlockStart.size();
        match.block_end.begin = input.find(BlockEnd, match.block_start.end);
        if (match.block_end.begin == string::npos)
        {
            return -1;
        }
        match.block_end.end = match.block_end.begin + BlockEnd.size();
        match.command.begin = input.find_first_not_of(' ', match.block_start.end);
        for (size_t ii = 0; ii < commands.size(); ++ii)
        {
            const string& command = commands[ii];
            match.command.end = match.command.begin + command.size();
            if (input.compare(match.command.begin, block_match_t::size(match.command), command) == 0)
            {
                match.content.begin = input.find_first_not_of(' ', match.command.end);
                match.content.end = input.find_last_not_of(' ', match.block_end.begin);
                return ssize_t(ii);
            }
        }
    }
    return -1;
}

ControlBlock::ptr ControlBlock::TryLoad(const string& input, size_t& offset, const XResources::ptr& resources)
{
    cmatch match;
    if (input.compare(offset, BlockStart.size(), BlockStart) == 0)
    {
        size_t start = offset + BlockStart.size();
        size_t end = input.find(BlockEnd, start);
        if (end == string::npos)
        {
            return nullptr;
        }
        // offset = end + BlockEnd.size();
        string block = input.substr(start, end - start);
        tools::trim(block);
        if (block.find("block") == 0)
        {
            return make_shared<BlockBlock>(block, input, offset, resources);
        }
        else if (block.find("extends") == 0)
        {
            return make_shared<ExtendBlock>(block, input, offset, resources);
        }
        else if (block.find("if") == 0)
        {
            return make_shared<IfBlock>(block, input, offset, resources);
        }
        else if (block.find("for") == 0)
        {
            return make_shared<ForBlock>(block, input, offset, resources);
        }
        throw Error("unknown control block: "s + block);
    }
    else
    {
        return nullptr;
    }
}

string ControlBlock::GetContent(const string& input, const block_match_t& start_block, const block_match_t& end_block)
{
    return string(input.begin() + ssize_t(start_block.block_end.end), input.begin() + ssize_t(end_block.block_start.begin));
}

//
// RootBlock
//

RootBlock::ptr RootBlock::Compile(const string & input, const XResources::ptr& resources)
{
    RootBlock::ptr root = make_shared<tools::make_shared_enabler<RootBlock>>(resources);
    root->m_children = TreeBlock::Load(input, resources);
    return root;
}

RootBlock::ptr RootBlock::CompileFile(const string& path, const XResources::ptr& resources)
{
    ifstream stream(path);
    return Compile(string(istreambuf_iterator<char>(stream), istreambuf_iterator<char>()), resources);
}

RootBlock::ptr RootBlock::CompileResource(const string& key, const XResources::ptr& resources)
{
    return Compile(resources->getString(key), resources);
}

//
// TreeBlock
//

string TreeBlock::process(const xdict& context)
{
    return m_children.process(context);
}

BlockList TreeBlock::Load(const string & input, const XResources::ptr& resources)
{
    BlockList lst;
    size_t offset = 0;
    ExtendBlock::ptr extends;
    size_t prev_pos = 0;
    size_t start_pos = 0;
    while (offset < input.size())
    {
        if (offset >= start_pos)
        {
            start_pos = prev_pos = offset;
        }
        start_pos = offset = input.find('{', start_pos);
        if (offset == string::npos)
        {
            // end of file reached, add a TextBlock with remaining data
            lst.push_back(make_shared<TextBlock>(string(input.begin() + ssize_t(prev_pos), input.end())));
            break;
        }
        BaseBlock::ptr child = nullptr;
        if (child = RenderBlock::TryLoad(input, offset, resources); child)
        {
            // add a TextBlock with previous data
            if (start_pos - prev_pos > 0)
            {
                lst.push_back(make_shared<TextBlock>(string(input.begin() + ssize_t(prev_pos), input.begin() + ssize_t(start_pos))));
            }
            lst.push_back(child);
        }
        else if (child = ControlBlock::TryLoad(input, offset, resources); child)
        {
            // add a TextBlock with previous data
            if (start_pos - prev_pos > 0)
            {
                lst.push_back(make_shared<TextBlock>(string(input.begin() + ssize_t(prev_pos), input.begin() + ssize_t(start_pos))));
            }
            ExtendBlock::ptr extend_block = dynamic_pointer_cast<ExtendBlock>(child);
            if (extend_block)
            {
                if (extends)
                {
                    throw Error("Multiple extends not allowed");
                }
                extends = extend_block;
            }
            else
            {
                lst.push_back(child);
            }
        }
        else
        {
            ++start_pos;
        }
    }
    if (extends)
    {
        lst = extends->extend(lst);
    }
    return lst;
}

//
// IfBlock
//

IfBlock::IfBlock(const string & /*block*/, const string & input, size_t & offset, const XResources::ptr& resources):
    ControlBlock(resources)
{
    int level = 0;
    block_match_t bmatch, if_match;
    bmatch.block_end.end = offset;

    auto push_if_children = [this, &input, &resources](block_match_t& previous, const block_match_t& current)
    {
        if (!previous.is_set())
        {
            throw Error("if missing");
        }

        string content = GetContent(input, previous, current);
        string expression = block_match_t::get(input, previous.content);
        tools::trim(expression);
        m_ifChildren.push_back({Expression::Load(expression), TreeBlock::Load(content, resources)});
        previous = current;
    };

    bool is_else = false;
    do
    {
        static const vector<string> block_lookups = { "if", "elif", "else", "endif" };
        ssize_t index = NextBlockOf(input, block_lookups, bmatch.block_end.end, bmatch);
        // next for lookup
        if (index == 0)
        {
            if (level++ == 0)
            {
                // if
                if_match = bmatch;
            }
        }
        else if (index == 1)
        {
            if (level == 1)
            {
                // else if
                push_if_children(if_match, bmatch);
            }
        }
        else if (index == 2)
        {
            if (level == 1)
            {
                // else
                push_if_children(if_match, bmatch);
                is_else = true;
            }
        }
        else if (index == 3)
        {
            // endif
            if (--level == 0)
            {
                if (!if_match.is_set())
                {
                    throw Error("if missing");
                }
                string content = GetContent(input, if_match, bmatch);
                if (!is_else)
                {
                    string expression = block_match_t::get(input, if_match.content);
                    tools::trim(expression);
                    m_ifChildren.push_back({Expression::Load(expression), TreeBlock::Load(content, resources)});
                }
                else
                {
                    m_children = TreeBlock::Load(content, resources);
                }
            }
        }
        else
        {
            throw Error("endif missing");
        }
    } while (level > 0);
    offset = bmatch.block_end.end;
}

string IfBlock::process(const xdict& context)
{
    for (auto& item : m_ifChildren)
    {
        try {
            xvar check = item.first->eval(context, m_resources);
            if (check.is<bool>())
            {
                if (check.get<bool>() == true)
                {
                    return item.second.process(context);
                }
            }
            else if (!check.empty())
            {
                return item.second.process(context);
            }
        } catch (const Expression::Error&) {}
    }
    // else
    return m_children.process(context);
}

//
// ForBlock
//

ForBlock::ForBlock(const string & block, const string & input, size_t & offset, const XResources::ptr& resources):
    ControlBlock(resources)
{
    block_match_t bmatch, for_bmatch, endfor_bmatch;
    bmatch.block_end.end = offset;
    int level = 0;
    do {
        static const vector<string> block_lookups = { "for", "endfor" };
        ssize_t index = NextBlockOf(input, block_lookups, bmatch.block_end.end, bmatch);
        // next for lookup
        if (index == 0)
        {
            if (level++ == 0)
            {
                for_bmatch = bmatch;
            }
        }
        else if (index == 1)
        {
            if (--level == 0)
            {
                endfor_bmatch = bmatch;
            }
        }
        else
        {
            throw Error("endfor missing");
        }
    } while (level > 0);

    offset = endfor_bmatch.block_end.end;
    m_children = TreeBlock::Load(GetContent(input, for_bmatch, endfor_bmatch), resources);
    static const regex subcontext_regex(R"(for\s+(.*)\s+in\s+(.*))");
    cmatch for_match;
    if (regex_match(block.c_str(), for_match, subcontext_regex))
    {
        m_keys = tools::split(for_match[1], ',', true);
        m_iterated = for_match[2];
    }
    else
    {
        throw Error("cannot parse block: " + block);
    }
}

string ForBlock::process(const xdict& context)
{
    string result;
    xdict for_context = context;
    xvar iterable;
    try {
        iterable = for_context.at(m_iterated);
    }
    catch (const out_of_range&) {
        throw XException("Undefined variable: " + m_iterated);
    }
    if (iterable.is<xdict>())
    {
        if (m_keys.size() > 2)
        {
            throw Error("An dict cannot must have at most two keys (" + m_iterated + ")");
        }
        string key = m_keys[0];
        tools::trim(key);
        string value;
        if (m_keys.size() == 2)
        {
            value = m_keys[1];
            tools::trim(value);
        }
        for (const auto& item : iterable.get<xdict>())
        {
            if (value.size())
            {
                for_context[key] = item.first;
                for_context[value] = item.second;
            }
            else
            {
                for_context[key] = item.second; // key is value
            }
            result += m_children.process(for_context);
        }
    }
    else if (iterable.is<xlist>())
    {
        if (m_keys.size() != 1)
        {
            throw runtime_error("An dict cannot must have only one keys (" + m_iterated + ")");
        }
        string value = m_keys[0];
        tools::trim(value);
        int index = 0;
        auto& array = iterable.get<xlist>();
        for_context["loop"] = xdict{
            {"size", static_cast<int>(array.size())}
        };
        auto& loop = for_context["loop"].get<xdict>();
        for (auto& item : array)
        {
            loop["index"] = index;
            loop["last"] = index + 1 >= static_cast<int>(array.size());
            for_context[value] = item;
            result += m_children.process(for_context);
            ++index;
        }
    }
    else
    {
        // is this an error ??? An empty json-loaded array might result in this...
        // throw runtime_error(m_iterated + " is not iterable");
    }
    return result;
}

//
// BlockBlock
//

BlockBlock::BlockBlock(const string& block, const string& input, size_t& offset, const XResources::ptr& resources):
    ControlBlock(resources)
{
    const regex extract(R"(\w+\s+["']?(.+)["']?)");
    smatch match;
    if (!regex_match(block, match, extract))
    {
        throw Error("Failed to extract block block");
    }
    m_id = match[1];

    block_match_t bmatch, block_bmatch;
    bmatch.block_end.end = offset;
    int level = 0;
    do {
        auto index = NextBlockOf(input, { "block", "endblock" }, bmatch.block_end.end, bmatch);
        if (index == 0)
        {
            // block
            if (level++ == 0)
            {
                block_bmatch = bmatch;
            }
        }
        else
        {
            // endblock
            --level;
        }
    } while(level > 0);
    offset = bmatch.block_end.end;
    m_children = TreeBlock::Load(GetContent(input, block_bmatch, bmatch), resources);
}

string BlockBlock::process(const xdict & context)
{
    string result;
    xdict child_ctx = context;
    temp::TemplateExpression::RenderFunctionMap& function_map = temp::TemplateExpression::RenderFunctions;
    string super_key = fmt::format("super#{}", static_cast<void*>(this));
    function_map[super_key] = [this, child_ctx](const xlist& /*args*/, const xdict& /*context*/, const XResources::ptr& /*res*/) {
        //BlockBlock::ptr super = m_super.lock();
        BlockBlock::ptr super = m_super;
        string res;
        if (super)
        {
            res = super->process(child_ctx);
        }
        return res;
    };
    child_ctx["$super_key"] = super_key;
    for (auto& item : m_children)
    {
        result += item->process(child_ctx);
    }
    function_map.erase(super_key);
    return result;
}

//
// ExtendBlock
//

ExtendBlock::ExtendBlock(const string& block, const string& input, size_t& offset, const XResources::ptr& resources):
    ControlBlock(resources)
{
    const regex extract(R"(\w+\s+["']?(.+)["']?)");
    smatch match;
    if (!regex_match(block, match, extract))
    {
        throw Error("Failed to extract extends block");
    }
    m_key = match[1];

    block_match_t bmatch;
    NextBlockOf(input, { "extends" }, offset, bmatch);
    offset = bmatch.block_end.end;
}

template <typename T, typename Iterable>
typename T::ptr find_next(Iterable& begin, const Iterable& end)
{
    for (; begin != end; begin++)
    {
        typename T::ptr found = dynamic_pointer_cast<T>(*begin);
        if (found)
        {
            begin++;
            return found;
        }
    }
    return nullptr;
}

BlockList ExtendBlock::extend(const BlockList& child_lst)
{
    BlockList parent_lst;
    BlockList new_lst;
    std::any parent = m_resources->get(m_key);
    try {
        parent_lst = std::any_cast<RootBlock::ptr>(parent)->m_children;
    } catch(const std::bad_any_cast&) {
        parent_lst = RootBlock::Compile(std::any_cast<string>(parent), m_resources)->m_children;
    }
    for (const auto& block : parent_lst)
    {
        BlockBlock::ptr block_block = dynamic_pointer_cast<BlockBlock>(block);
        if (block_block)
        {
            BlockList::const_iterator begin = child_lst.begin();
            BlockList::const_iterator end = child_lst.end();
            BlockBlock::ptr child_block = nullptr;
            while (begin != end)
            {
                child_block = find_next<BlockBlock>(begin, end);
                if (child_block && child_block->id() == block_block->id())
                {
                    break;
                }
                child_block = nullptr;
            }
            if (child_block)
            {
                child_block->setSuper(block_block);
                new_lst.push_back(child_block);
                continue;
            }
        }
        new_lst.push_back(block);
    }
    return new_lst;
}
