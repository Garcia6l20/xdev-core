/**
 * @file xdev-template-blocks.hpp
 * @autor Garcia Sylvain
 */
#pragma once

#include <xdev/xdev-template-export.hpp>

#include <xdev/xdev-variant.hpp>
#include <xdev/xdev-resources.hpp>
#include <xdev/xdev-template-expressions.hpp>

namespace xdev::temp {

class Error : public XException {
public:
    using XException::XException;
};

/**
 * @brief BaseBlock
 */
class BaseBlock
{
public:
    using ptr = shared_ptr<BaseBlock>;
    virtual string process(const xdict& context) = 0;
protected:
    BaseBlock(const XResources::ptr& resources);
    static bool ExtractFunction(const string& raw_content, string& function_name, vector<string>& args);
    const XResources::ptr m_resources;
    virtual ~BaseBlock();
private:
    BaseBlock() = delete;
};
class BlockList : public vector<BaseBlock::ptr>
{
public:
    using vector<BaseBlock::ptr>::vector;
    inline string process(const xdict& context)
    {
        string result;
        for (auto& item : *this)
        {
            result += item->process(context);
        }
        return result;
    }
};

class TreeBlock : public BaseBlock
{
public:
    virtual string process(const xdict& context) override;
protected:
    using BaseBlock::BaseBlock;
    BlockList m_children;
    static BlockList Load(const string& input, const XResources::ptr& resources);
    friend class ForBlock;
    friend class IfBlock;
    friend class BlockBlock;
    friend class ExtendBlock;
};

class XDEV_TEMPLATE_EXPORT RootBlock : public TreeBlock
{
public:
    using ptr = shared_ptr<RootBlock>;
    static RootBlock::ptr Compile(const string& input, const XResources::ptr& resources = nullptr);
    static RootBlock::ptr CompileFile(const string& path, const XResources::ptr& resources = nullptr);
    static RootBlock::ptr CompileResource(const string& key, const XResources::ptr& resources);
    inline string render(const xvar& context = xdict{}) {
        return process(context.get<xdict>());
    }
private:
    using TreeBlock::TreeBlock;
    friend struct tools::make_shared_enabler<RootBlock>;
};

/**
 * A simple block containing text data
 */
class TextBlock : public BaseBlock
{
public:
    TextBlock(const string& content, const XResources::ptr& resources = nullptr) :
        BaseBlock(resources),
        m_content(content)
    {
    }
    virtual string process(const xdict& /*context*/) override
    {
        return m_content;
    }
private:
    string m_content;
};

/**
 * A simple rendering block
 * {@code {{ my_var | my_pipe }}}
 */
class RenderBlock : public BaseBlock
{
    Expression::ptr m_expression;
public:
    static string BlockStart;
    static string BlockEnd;
    RenderBlock(const string& content, const XResources::ptr& resources);
    using ptr = shared_ptr<RenderBlock>;
    static ptr TryLoad(const string& input, size_t& offset, const XResources::ptr& resources);
    virtual string process(const xdict& context) override;
};

/**
 * A simple control block
 * {@code {% my_control %}}
 */
class ControlBlock : public TreeBlock
{
public:
    static string BlockStart;
    static string BlockEnd;
    using ptr = shared_ptr<ControlBlock>;
    static ptr TryLoad(const string& input, size_t& offset, const XResources::ptr& resources);
protected:
    using TreeBlock::TreeBlock;
    typedef struct block_match_s {
        typedef struct
        {
            size_t begin;
            size_t end;
        } bounds_t;
        bounds_t block_start;
        bounds_t block_end;
        bounds_t command;
        bounds_t content;
        static const size_t npos = size_t(-1);
        static inline string get(const string& input, const bounds_t& bounds)
        {
            return string(input.begin() + ssize_t(bounds.begin), input.begin() + ssize_t(bounds.end));
        }
        static inline size_t size(const bounds_t& bounds)
        {
            if (bounds.end < bounds.begin)
                return npos;
            return bounds.end - bounds.begin;
        }
        static inline void reset(bounds_t& bounds)
        {
            bounds.begin = npos;
            bounds.end = npos;
        }
        static inline bool is_set(const bounds_t& bounds)
        {
            return bounds.begin != npos && bounds.end != npos;
        }
        block_match_s() {
            reset();
        }
        inline void reset()
        {
            reset(command);
            reset(block_start);
            reset(block_end);
            reset(content);
        }
        inline bool is_set() const
        {
            return is_set(command) && is_set(block_start) && is_set(block_end);
        }
    } block_match_t;
    static string GetContent(const string& input, const block_match_t& start_block, const block_match_t& end_block);
    static ssize_t NextBlockOf(const string& input, const vector<string>& commands, size_t offset, block_match_t& match);
};

/**
 * An if block
 * {@code {% if ... %}...{% elif ... %}...{% else %}...{% endif %}}
 */
class IfBlock : public ControlBlock
{
public:
    IfBlock(const string& block, const string& input, size_t& offset, const XResources::ptr& resources);
    virtual string process(const xdict& context) override;
private:
    vector<pair<Expression::ptr, BlockList>> m_ifChildren;
};

/**
 * A for block
 * {@code {% for ... in ... %}...{% elfor %}...{% endfor %}}
 */
class ForBlock : public ControlBlock
{
    string m_iterated;
    vector<string> m_keys;
public:
    ForBlock(const string& block, const string& input, size_t& offset, const XResources::ptr& resources);
    virtual string process(const xdict& context) override;
};

/**
 * A block block, used in template overriding
 * {@code {% block my_block %}...{% end my_block %}}
 */
class BlockBlock : public ControlBlock
{
public:
    using ptr = shared_ptr<BlockBlock>;
    using wptr = weak_ptr<BlockBlock>;
    BlockBlock(const string& block, const string& input, size_t& offset, const XResources::ptr& resources);
    inline string id() const { return m_id; }
    inline void setSuper(const BlockBlock::ptr& super) { m_super = super; }
    virtual string process(const xdict& context) override;
private:
    string m_id;
    BlockBlock::ptr m_super;
};

/**
* A block block, used in template overriding
* {@code {% extends 'template' %}}
*/
class ExtendBlock : public ControlBlock
{
public:
    using ptr = shared_ptr<ExtendBlock>;
    ExtendBlock(const string& block, const string& input, size_t& offset, const XResources::ptr& resources);
    BlockList extend(const BlockList& child_lst);
private:
    string m_key;
};

} // namespace xdev::temp
