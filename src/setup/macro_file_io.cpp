//------------------------------------------------------------------------------
// UniKey Vietnamese Input Method
// Copyright (C) 2000-2005 Pham Kim Long
// Copyright (C) 2026 Hoang Duy Tran & AI Agent
// Contact:
//   unikey@gmail.com
//   hoangduytran1960@googlemail.com
//   UniKey project: http://unikey.org
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//------------------------------------------------------------------------------

#include <cctype>
#include <string>
#include <vector>

#include <glib.h>

#include "keycons.h"
#include "macro_file_io.h"
#include "vnconv.h"

namespace {

enum class MacroFileFormat {
    NATIVE,
    YAML,
    PLIST,
};

struct MacroFileEntry {
    std::string trigger;
    std::string replace;
};

static GQuark macro_file_error_quark()
{
    return g_quark_from_static_string("unikey-macro-file-error");
}

static std::string trim_copy(const std::string& text)
{
    size_t start = 0;
    while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start]))) {
        start++;
    }

    size_t end = text.size();
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
        end--;
    }

    return text.substr(start, end - start);
}

static int leading_spaces(const std::string& text)
{
    int indent = 0;
    while (indent < static_cast<int>(text.size()) && text[indent] == ' ') {
        indent++;
    }
    return indent;
}

static bool starts_with(const std::string& text, const char* prefix)
{
    const std::string prefix_text(prefix);
    return text.compare(0, prefix_text.size(), prefix_text) == 0;
}

static std::string lowercase_ascii(const char* text)
{
    std::string result = (text != NULL) ? text : "";
    for (size_t i = 0; i < result.size(); i++) {
        result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
    }
    return result;
}

static MacroFileFormat detect_macro_file_format(const char* filename)
{
    const std::string lower_name = lowercase_ascii(filename);
    if (lower_name.size() >= 5 && lower_name.compare(lower_name.size() - 5, 5, ".yaml") == 0) {
        return MacroFileFormat::YAML;
    }
    if (lower_name.size() >= 4 && lower_name.compare(lower_name.size() - 4, 4, ".yml") == 0) {
        return MacroFileFormat::YAML;
    }
    if (lower_name.size() >= 6 && lower_name.compare(lower_name.size() - 6, 6, ".plist") == 0) {
        return MacroFileFormat::PLIST;
    }
    return MacroFileFormat::NATIVE;
}

static bool convert_vnstandard_to_utf8(const StdVnChar* input, std::string& output)
{
    if (input == NULL) {
        output.clear();
        return true;
    }

    char buffer[MAX_MACRO_TEXT_LEN * 3 + 1];
    int inLen = -1;
    int maxOutLen = static_cast<int>(sizeof(buffer));
    int ret = VnConvert(CONV_CHARSET_VNSTANDARD, CONV_CHARSET_XUTF8,
                        reinterpret_cast<UKBYTE*>(const_cast<StdVnChar*>(input)),
                        reinterpret_cast<UKBYTE*>(buffer),
                        &inLen, &maxOutLen);
    if (ret != 0) {
        return false;
    }

    output.assign(buffer);
    return true;
}

static bool collect_macro_entries(CMacroTable* macro,
                                  std::vector<MacroFileEntry>& entries,
                                  GError** error)
{
    entries.clear();
    for (int i = 0; i < macro->getCount(); i++) {
        MacroFileEntry entry;
        bool has_trigger = convert_vnstandard_to_utf8(macro->getKey(i), entry.trigger);
        bool has_replace = convert_vnstandard_to_utf8(macro->getText(i), entry.replace);
        if (!has_trigger || !has_replace) {
            g_set_error(error,
                        macro_file_error_quark(),
                        1,
                        "Failed to convert macro %d to UTF-8.",
                        i + 1);
            return false;
        }
        entries.push_back(entry);
    }

    return true;
}

static bool add_entries_to_macro(const std::vector<MacroFileEntry>& entries,
                                 CMacroTable* macro,
                                 GError** error)
{
    macro->resetContent();
    for (size_t i = 0; i < entries.size(); i++) {
        const MacroFileEntry& entry = entries[i];
        if (entry.trigger.empty()) {
            continue;
        }

        if (macro->addItem(entry.trigger.c_str(), entry.replace.c_str(), CONV_CHARSET_XUTF8) < 0) {
            g_set_error(error,
                        macro_file_error_quark(),
                        1,
                        "Failed to import macro '%s'.",
                        entry.trigger.c_str());
            return false;
        }
    }

    return true;
}

static std::string strip_plain_yaml_comment(const std::string& value)
{
    for (size_t i = 0; i < value.size(); i++) {
        if (value[i] == '#' && (i == 0 || std::isspace(static_cast<unsigned char>(value[i - 1])))) {
            return trim_copy(value.substr(0, i));
        }
    }
    return trim_copy(value);
}

static bool parse_yaml_double_quoted(const std::string& value, std::string& output)
{
    if (value.size() < 2 || value.front() != '"' || value.back() != '"') {
        return false;
    }

    const std::string inner = value.substr(1, value.size() - 2);
    gchar* unescaped = g_strcompress(inner.c_str());
    output.assign(unescaped != NULL ? unescaped : "");
    g_free(unescaped);
    return true;
}

static bool parse_yaml_single_quoted(const std::string& value, std::string& output)
{
    if (value.size() < 2 || value.front() != '\'' || value.back() != '\'') {
        return false;
    }

    output.clear();
    for (size_t i = 1; i + 1 < value.size(); i++) {
        if (value[i] == '\'' && i + 2 < value.size() && value[i + 1] == '\'') {
            output.push_back('\'');
            i++;
            continue;
        }
        output.push_back(value[i]);
    }
    return true;
}

static std::string join_yaml_literal_lines(const std::vector<std::string>& lines)
{
    std::string result;
    for (size_t i = 0; i < lines.size(); i++) {
        if (i > 0) {
            result.push_back('\n');
        }
        result += lines[i];
    }
    return result;
}

static std::string join_yaml_folded_lines(const std::vector<std::string>& lines)
{
    std::string result;
    bool previous_blank = true;
    for (size_t i = 0; i < lines.size(); i++) {
        const bool is_blank_line = lines[i].empty();
        if (is_blank_line) {
            result.push_back('\n');
            previous_blank = true;
            continue;
        }

        if (!result.empty()) {
            result.push_back(previous_blank ? '\n' : ' ');
        }
        result += lines[i];
        previous_blank = false;
    }
    return result;
}

static bool parse_yaml_block_scalar(const std::vector<std::string>& lines,
                                    size_t& index,
                                    int property_indent,
                                    const std::string& header,
                                    std::string& output)
{
    const bool is_folded = !header.empty() && header[0] == '>';
    const bool strip_trailing_newline = header.find('-') != std::string::npos;

    std::vector<std::string> block_lines;
    int block_indent = -1;
    size_t cursor = index + 1;

    while (cursor < lines.size()) {
        const std::string& line = lines[cursor];
        const int indent = leading_spaces(line);
        const std::string content = line.substr(indent);
        const bool is_blank_line = trim_copy(line).empty();

        if (!is_blank_line && indent <= property_indent) {
            break;
        }

        if (is_blank_line) {
            if (block_indent == -1) {
                block_lines.push_back("");
            } else {
                block_lines.push_back(indent >= block_indent ? line.substr(block_indent) : std::string());
            }
            cursor++;
            continue;
        }

        if (block_indent == -1) {
            block_indent = indent;
        }

        if (indent < block_indent) {
            break;
        }

        block_lines.push_back(line.substr(block_indent));
        cursor++;
    }

    if (strip_trailing_newline) {
        while (!block_lines.empty() && block_lines.back().empty()) {
            block_lines.pop_back();
        }
    }

    output = is_folded ? join_yaml_folded_lines(block_lines) : join_yaml_literal_lines(block_lines);
    if (!strip_trailing_newline && !block_lines.empty()) {
        output.push_back('\n');
    }

    index = (cursor == 0) ? 0 : cursor - 1;
    return true;
}

static bool parse_yaml_scalar(const std::vector<std::string>& lines,
                              size_t& index,
                              int property_indent,
                              const std::string& raw_value,
                              std::string& output,
                              GError** error)
{
    const std::string value = trim_copy(raw_value);
    if (value.empty()) {
        output.clear();
        return true;
    }

    if (value[0] == '|' || value[0] == '>') {
        return parse_yaml_block_scalar(lines, index, property_indent, value, output);
    }

    if (parse_yaml_double_quoted(value, output)) {
        return true;
    }

    if (parse_yaml_single_quoted(value, output)) {
        return true;
    }

    if ((value.front() == '"' && value.back() != '"') ||
        (value.front() == '\'' && value.back() != '\'')) {
        g_set_error(error,
                    macro_file_error_quark(),
                    1,
                    "Invalid YAML quoted scalar near line %u.",
                    static_cast<unsigned>(index + 1));
        return false;
    }

    output = strip_plain_yaml_comment(value);
    return true;
}

static bool parse_yaml_entries(const gchar* text,
                               std::vector<MacroFileEntry>& entries,
                               GError** error)
{
    entries.clear();

    std::vector<std::string> lines;
    std::string input = text != NULL ? text : "";
    size_t start = 0;
    while (start <= input.size()) {
        const size_t end = input.find('\n', start);
        std::string line = (end == std::string::npos)
            ? input.substr(start)
            : input.substr(start, end - start);
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }

    for (size_t i = 0; i < lines.size(); i++) {
        const std::string trimmed = trim_copy(lines[i]);
        if (trimmed.empty() || trimmed[0] == '#') {
            continue;
        }
        if (!starts_with(trimmed, "- ")) {
            continue;
        }

        const int item_indent = leading_spaces(lines[i]);
        MacroFileEntry entry;
        bool has_trigger = false;
        bool has_replace = false;

        std::string first_property = trim_copy(trimmed.substr(2));
        size_t colon = first_property.find(':');
        if (colon != std::string::npos) {
            const std::string key = trim_copy(first_property.substr(0, colon));
            const std::string raw_value = first_property.substr(colon + 1);
            if (key == "trigger") {
                has_trigger = parse_yaml_scalar(lines, i, item_indent + 2, raw_value, entry.trigger, error);
                if (!has_trigger) {
                    return false;
                }
            } else if (key == "replace") {
                has_replace = parse_yaml_scalar(lines, i, item_indent + 2, raw_value, entry.replace, error);
                if (!has_replace) {
                    return false;
                }
            }
        }

        size_t cursor = i + 1;
        for (; cursor < lines.size(); cursor++) {
            const std::string trimmed_line = trim_copy(lines[cursor]);
            if (trimmed_line.empty() || trimmed_line[0] == '#') {
                continue;
            }

            const int indent = leading_spaces(lines[cursor]);
            if (indent <= item_indent && starts_with(trimmed_line, "- ")) {
                break;
            }
            if (indent <= item_indent) {
                break;
            }

            const size_t property_colon = trimmed_line.find(':');
            if (property_colon == std::string::npos) {
                continue;
            }

            const std::string property_name = trim_copy(trimmed_line.substr(0, property_colon));
            const std::string raw_value = trimmed_line.substr(property_colon + 1);
            if (property_name == "trigger") {
                if (!parse_yaml_scalar(lines, cursor, indent, raw_value, entry.trigger, error)) {
                    return false;
                }
                has_trigger = true;
                continue;
            }
            if (property_name == "replace") {
                if (!parse_yaml_scalar(lines, cursor, indent, raw_value, entry.replace, error)) {
                    return false;
                }
                has_replace = true;
            }
        }

        if (has_trigger && has_replace) {
            entries.push_back(entry);
        }
        i = cursor == 0 ? cursor : cursor - 1;
    }

    return true;
}

struct PlistParseState {
    std::vector<MacroFileEntry>* entries;
    std::string current_text;
    std::string current_key;
    std::string current_element;
    MacroFileEntry current_entry;
    bool is_in_dict;
};

static void plist_start_element(GMarkupParseContext* context,
                                const gchar* element_name,
                                const gchar** attribute_names,
                                const gchar** attribute_values,
                                gpointer user_data,
                                GError** error)
{
    (void)context;
    (void)attribute_names;
    (void)attribute_values;
    (void)error;

    PlistParseState* state = reinterpret_cast<PlistParseState*>(user_data);
    state->current_element = element_name;
    state->current_text.clear();
    if (g_strcmp0(element_name, "dict") == 0) {
        state->is_in_dict = true;
        state->current_key.clear();
        state->current_entry = MacroFileEntry();
    }
}

static void plist_end_element(GMarkupParseContext* context,
                              const gchar* element_name,
                              gpointer user_data,
                              GError** error)
{
    (void)context;
    (void)error;

    PlistParseState* state = reinterpret_cast<PlistParseState*>(user_data);
    if (g_strcmp0(element_name, "key") == 0) {
        state->current_key = state->current_text;
    } else if (g_strcmp0(element_name, "string") == 0) {
        if (state->current_key == "shortcut") {
            state->current_entry.trigger = state->current_text;
        } else if (state->current_key == "phrase") {
            state->current_entry.replace = state->current_text;
        }
    } else if (g_strcmp0(element_name, "dict") == 0) {
        if (!state->current_entry.trigger.empty()) {
            state->entries->push_back(state->current_entry);
        }
        state->is_in_dict = false;
        state->current_key.clear();
        state->current_entry = MacroFileEntry();
    }

    state->current_text.clear();
    state->current_element.clear();
}

static void plist_text(GMarkupParseContext* context,
                       const gchar* text,
                       gsize text_len,
                       gpointer user_data,
                       GError** error)
{
    (void)context;
    (void)error;

    PlistParseState* state = reinterpret_cast<PlistParseState*>(user_data);
    if (state->current_element == "key" || state->current_element == "string") {
        state->current_text.append(text, text_len);
    }
}

static bool parse_plist_entries(const gchar* text,
                                std::vector<MacroFileEntry>& entries,
                                GError** error)
{
    entries.clear();

    GString* sanitized = g_string_new(NULL);
    std::string input = text != NULL ? text : "";
    size_t start = 0;
    while (start <= input.size()) {
        const size_t end = input.find('\n', start);
        std::string line = (end == std::string::npos)
            ? input.substr(start)
            : input.substr(start, end - start);
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        const std::string trimmed = trim_copy(line);
        const bool is_xml_declaration = starts_with(trimmed, "<?xml");
        const bool is_doctype = starts_with(trimmed, "<!DOCTYPE");
        if (!is_xml_declaration && !is_doctype) {
            g_string_append(sanitized, line.c_str());
            g_string_append_c(sanitized, '\n');
        }

        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }

    PlistParseState state;
    state.entries = &entries;
    state.is_in_dict = false;

    GMarkupParser parser = {};
    parser.start_element = plist_start_element;
    parser.end_element = plist_end_element;
    parser.text = plist_text;

    GMarkupParseContext* context = g_markup_parse_context_new(&parser,
                                                              static_cast<GMarkupParseFlags>(0),
                                                              &state,
                                                              NULL);
    const gboolean ok = g_markup_parse_context_parse(context, sanitized->str, -1, error);
    gboolean ended = FALSE;
    if (ok) {
        ended = g_markup_parse_context_end_parse(context, error);
    }
    g_markup_parse_context_free(context);
    g_string_free(sanitized, TRUE);

    return ok && ended;
}

static std::string quote_yaml_double(const std::string& value)
{
    std::string result = "\"";
    for (size_t i = 0; i < value.size(); i++) {
        const unsigned char ch = static_cast<unsigned char>(value[i]);
        switch (ch) {
            case '\\': result += "\\\\"; break;
            case '\"': result += "\\\""; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:
                if (ch < 0x20) {
                    gchar* escaped = g_strdup_printf("\\x%02X", ch);
                    result += escaped;
                    g_free(escaped);
                } else {
                    result.push_back(static_cast<char>(ch));
                }
                break;
        }
    }
    result += "\"";
    return result;
}

static void append_yaml_block(GString* content, const std::string& value)
{
    g_string_append(content, "|-\n");

    size_t start = 0;
    while (start <= value.size()) {
        const size_t end = value.find('\n', start);
        const std::string line = (end == std::string::npos)
            ? value.substr(start)
            : value.substr(start, end - start);
        g_string_append(content, "      ");
        g_string_append(content, line.c_str());
        g_string_append_c(content, '\n');
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }
}

static bool build_yaml_content(CMacroTable* macro, std::string& output, GError** error)
{
    std::vector<MacroFileEntry> entries;
    if (!collect_macro_entries(macro, entries, error)) {
        return false;
    }

    GString* content = g_string_new("matches:\n");
    for (size_t i = 0; i < entries.size(); i++) {
        const MacroFileEntry& entry = entries[i];
        g_string_append(content, "  - trigger: ");
        g_string_append(content, quote_yaml_double(entry.trigger).c_str());
        g_string_append_c(content, '\n');
        g_string_append(content, "    replace: ");
        if (entry.replace.find('\n') != std::string::npos) {
            append_yaml_block(content, entry.replace);
        } else {
            g_string_append(content, quote_yaml_double(entry.replace).c_str());
            g_string_append_c(content, '\n');
        }

        if (i + 1 < entries.size()) {
            g_string_append_c(content, '\n');
        }
    }

    output.assign(content->str, content->len);
    g_string_free(content, TRUE);
    return true;
}

static bool build_plist_content(CMacroTable* macro, std::string& output, GError** error)
{
    std::vector<MacroFileEntry> entries;
    if (!collect_macro_entries(macro, entries, error)) {
        return false;
    }

    GString* content = g_string_new("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    g_string_append(content, "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n");
    g_string_append(content, "<plist version=\"1.0\">\n");
    g_string_append(content, "<array>\n");

    for (size_t i = 0; i < entries.size(); i++) {
        gchar* escaped_phrase = g_markup_escape_text(entries[i].replace.c_str(), -1);
        gchar* escaped_shortcut = g_markup_escape_text(entries[i].trigger.c_str(), -1);

        g_string_append(content, "    <dict>\n");
        g_string_append(content, "        <key>phrase</key>\n");
        g_string_append_printf(content, "        <string>%s</string>\n", escaped_phrase);
        g_string_append(content, "        <key>shortcut</key>\n");
        g_string_append_printf(content, "        <string>%s</string>\n", escaped_shortcut);
        g_string_append(content, "    </dict>\n");

        g_free(escaped_phrase);
        g_free(escaped_shortcut);
    }

    g_string_append(content, "</array>\n");
    g_string_append(content, "</plist>\n");

    output.assign(content->str, content->len);
    g_string_free(content, TRUE);
    return true;
}

static bool load_yaml_file(const char* filename, CMacroTable* macro, GError** error)
{
    gchar* content = NULL;
    gsize content_length = 0;
    if (!g_file_get_contents(filename, &content, &content_length, error)) {
        return false;
    }

    std::vector<MacroFileEntry> entries;
    const bool parsed = parse_yaml_entries(content, entries, error);
    g_free(content);
    if (!parsed) {
        return false;
    }

    return add_entries_to_macro(entries, macro, error);
}

static bool load_plist_file(const char* filename, CMacroTable* macro, GError** error)
{
    gchar* content = NULL;
    gsize content_length = 0;
    if (!g_file_get_contents(filename, &content, &content_length, error)) {
        return false;
    }

    std::vector<MacroFileEntry> entries;
    const bool parsed = parse_plist_entries(content, entries, error);
    g_free(content);
    if (!parsed) {
        return false;
    }

    return add_entries_to_macro(entries, macro, error);
}

} // namespace

bool macro_table_load_any_format(const char* filename, CMacroTable* macro, GError** error)
{
    if (macro == NULL || filename == NULL) {
        g_set_error_literal(error,
                            macro_file_error_quark(),
                            1,
                            "Macro import received an invalid file path.");
        return false;
    }

    macro->init();
    switch (detect_macro_file_format(filename)) {
        case MacroFileFormat::YAML:
            return load_yaml_file(filename, macro, error);
        case MacroFileFormat::PLIST:
            return load_plist_file(filename, macro, error);
        case MacroFileFormat::NATIVE:
        default:
            if (macro->loadFromFile(filename)) {
                return true;
            }
            g_set_error(error,
                        macro_file_error_quark(),
                        1,
                        "Failed to open macro file '%s'.",
                        filename);
            return false;
    }
}

bool macro_table_write_any_format(const char* filename, CMacroTable* macro, GError** error)
{
    if (macro == NULL || filename == NULL) {
        g_set_error_literal(error,
                            macro_file_error_quark(),
                            1,
                            "Macro export received an invalid file path.");
        return false;
    }

    switch (detect_macro_file_format(filename)) {
        case MacroFileFormat::YAML: {
            std::string content;
            if (!build_yaml_content(macro, content, error)) {
                return false;
            }
            return g_file_set_contents(filename, content.c_str(), content.size(), error);
        }
        case MacroFileFormat::PLIST: {
            std::string content;
            if (!build_plist_content(macro, content, error)) {
                return false;
            }
            return g_file_set_contents(filename, content.c_str(), content.size(), error);
        }
        case MacroFileFormat::NATIVE:
        default:
            if (macro->writeToFile(filename)) {
                return true;
            }
            g_set_error(error,
                        macro_file_error_quark(),
                        1,
                        "Failed to write macro file '%s'.",
                        filename);
            return false;
    }
}