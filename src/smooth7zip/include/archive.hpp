#pragma once

#include <filesystem>
#include <stack>

#include <7zip/CPP/Common/MyCom.h>
#include <7zip/CPP/Windows/FileFind.h> // NWindows::NFile::NFind::CFileInfo

#include "format.hpp"

namespace smooth7zip {

auto set_large_page_mode() -> bool;

class archive {
public:
    enum class open_mode {
        none, // invalid mode
        read, // open existing archive
        write // add new files to new archive or existing archive
    };

    archive(const std::string _path = "",
            const open_mode _mode = open_mode::read,
            const format::format _format = format::format{});

    open_mode get_mode() const { return mode_; }

    std::string get_stem() const { return input_stem_; }

    std::uint32_t get_count() const;

    void add_directory(const std::string& _dir_path);

    void compress_to(const std::string& _out_path);

    void extract_to(const std::string& _out_dir_path);

private:
    CMyComPtr<IInStream> create_input_stream();

    CMyComPtr<IInArchive> open_in_archive(CMyComPtr<IInStream> _in_stream);

    std::string input_stem_;
    open_mode mode_;
    format::format format_;
    CMyComPtr<IInArchive> in_archive_ptr_;
    std::unique_ptr<archive> input_archive_ptr_;
    CObjectVector<NWindows::NFile::NFind::CFileInfo> to_archive_items_;
};

}; // namespace smooth7zip
