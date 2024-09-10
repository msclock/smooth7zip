#include "archive.hpp"

#include <7zip/CPP/7zip/Common/FileStreams.h>
#include <7zip/CPP/Windows/FileName.h> // NormalizeDirPathPrefix

#include "handler.hpp"

namespace smooth7zip {
auto set_large_page_mode() -> bool {
    return S_OK == SetLargePageMode() ? true : false;
}

archive::archive(const std::string _path, const open_mode _mode, const format::format _format)
    : mode_(_mode),
      format_(_format) {
    // verify input
    if (!std::filesystem::exists(_path)) {
        // throw when opening non-existing file in read mode
        if (mode_ == open_mode::read) {
            throw std::runtime_error("Archive not found: " + _path);
        }
        // No input file specified, so we are creating a totally new archive.
        return;
    }

    // check and set format
    if (!format_.is_valid()) {
        format_ = format::detect(_path);
    }

    if (!format_.is_valid()) {
        throw std::runtime_error("Invalid format archive detected: " + _path);
    }

    input_stem_ = _path;

    // Open archive

    if (mode_ == open_mode::read) {
        auto in_stream = create_input_stream();
        in_archive_ptr_ = open_in_archive(in_stream);
    }
    else {
        this->input_archive_ptr_ = std::make_unique<archive>(_path, open_mode::read, this->format_);
    }
    return;
}

std::uint32_t archive::get_count() const {
    std::uint32_t num_of_items = 0;
    if (S_OK != in_archive_ptr_->GetNumberOfItems(&num_of_items)) {
        throw std::runtime_error("Failed to get number of items");
    }
    return num_of_items;
}

void archive::add_directory(const std::string& _dir_path) {
    // iterate over directory
    NWindows::NFile::NFind::CFileInfo fi;

    auto to_archive_path = std::filesystem::directory_entry(_dir_path);
    std::stack<std::filesystem::directory_entry> to_archive_stack({to_archive_path});
    do {
        auto cur_archive_path = to_archive_stack.top();
        to_archive_stack.pop();
        for (const auto& to_archive_entry : std::filesystem::directory_iterator(cur_archive_path)) {
            if (to_archive_entry.is_regular_file()) {
                if (S_OK != fi.Find(to_archive_entry.path().c_str())) {
                    continue;
                }
                to_archive_items_.Add(fi);
            }
            else if (to_archive_entry.is_directory()) {
                to_archive_stack.push(to_archive_entry);
            }
        }
    }
    while (!to_archive_stack.empty());
}

void archive::compress_to(const std::string& _out_path) {
    // What if input path is same as output path? Consider updating existing archive.
    // if (std::filesystem::exists(_out_path) && !std::filesystem::remove(_out_path)) {
    //     throw std::runtime_error("Failed to remove existing output file: " + _out_path);
    // }

    // Update callback
    auto handler = smooth7zip::archive_handler();
    auto* update_callback_spec = new smooth7zip::archive_update_callback(handler);
    update_callback_spec->dir_items = &this->to_archive_items_;
    update_callback_spec->path_prefix = ""; // ?
    CMyComPtr<IArchiveUpdateCallback2> update_callback(update_callback_spec);

    // TODO: Update handler support
    // bool update_archive = std::filesystem::path(this->input_stem_) == _out_path;

    // Create output file stream
    auto archive_name = FString(_out_path.c_str());
    auto* out_file_stream_spec = new COutFileStream;
    if (!out_file_stream_spec->Create(archive_name, True)) {
        throw std::runtime_error("Failed to create output file stream: " + _out_path);
    }
    CMyComPtr<IOutStream> out_stream(out_file_stream_spec);

    // Out Archive
    CMyComPtr<IOutArchive> out_archive;
    auto format = smooth7zip::format::get_guid(this->format_);
    if (CreateObject(&format, &IID_IOutArchive, reinterpret_cast<void**>(&out_archive)) != S_OK) {
        throw std::runtime_error("Failed to create output archive");
    }

    // Set properties
    CMyComPtr<ISetProperties> set_properties;
    if (out_archive->QueryInterface(IID_ISetProperties, reinterpret_cast<void**>(&set_properties)) != S_OK) {
        throw std::runtime_error("Failed to get set properties handler");
    }

    const uint8_t num_prop = 1;
    const wchar_t* names[num_prop] = {L"x"}; // Compress level 9(utral)
    NWindows::NCOM::CPropVariant values[num_prop] = {static_cast<UINT32>(9)};
    if (set_properties->SetProperties(names, values, num_prop) != S_OK) {
        throw std::runtime_error("Failed to set properties");
    }

    // Update items in archive
    if (out_archive->UpdateItems(out_stream, this->to_archive_items_.Size(), update_callback) != S_OK) {
        // TODO: check update spec errors and throw exception
        throw std::runtime_error("Failed to update archive");
    }
}

void archive::extract_to(const std::string& _out_dir_path) {
    // Prepare extract callback
    auto handler = smooth7zip::archive_handler();
    auto* extract_callback_spec = new smooth7zip::archive_extract_callback(handler);
    auto extract_out_dir = FString(_out_dir_path.c_str());
    NWindows::NFile::NName::NormalizeDirPathPrefix(extract_out_dir);
    extract_callback_spec->in_archive = this->in_archive_ptr_;
    extract_callback_spec->extract_out_dir = extract_out_dir;
    CMyComPtr<IArchiveExtractCallback> extract_callback(extract_callback_spec);

    // TODO: Extract properties support
    const wchar_t* names[] = {L"mt"};
    const unsigned num_props = sizeof(names) / sizeof(names[0]);
    NWindows::NCOM::CPropVariant props[num_props] = {static_cast<UInt32>(1)};
    CMyComPtr<ISetProperties> set_props;
    this->in_archive_ptr_->QueryInterface(IID_ISetProperties, reinterpret_cast<void**>(&set_props));
    if (S_OK != set_props->SetProperties(names, props, num_props)) {
        throw std::runtime_error("Failed to set properties");
    }

    // extract archive
    if (S_OK != this->in_archive_ptr_->Extract(nullptr, static_cast<UINT32>(-1), false, extract_callback)) {
        // TODO: check extract spec errors and throw exception
        throw std::runtime_error("Failed to extract archive");
    }
}

CMyComPtr<IInStream> archive::create_input_stream() {
    CMyComPtr<IInStream> in_stream;
    auto* in_file_stream_spec = new CInFileStream; // TODO: MultiVolume support
    if (false == in_file_stream_spec->Open(FString(input_stem_.c_str()))) {
        throw std::runtime_error("Failed to open input file stream: " + input_stem_);
    }
    in_stream = in_file_stream_spec;
    return in_stream;
}

CMyComPtr<IInArchive> archive::open_in_archive(CMyComPtr<IInStream> _in_stream) {
    auto handler = smooth7zip::archive_handler();
    auto* open_callback_spec = new smooth7zip::archive_open_callback(handler);
    CMyComPtr<IArchiveOpenCallback> open_callback(open_callback_spec);
    open_callback_spec->passwd = "";  // TODO: password support
    const UInt64 scan_size = 1 << 23; // 8MB

    CMyComPtr<IInArchive> in_archive;
    auto format_id = smooth7zip::format::get_guid(this->format_);
    if (format_id == GUID{}) {
        throw std::runtime_error("Invalid format detected: " + this->format_.name);
    }
    if (S_OK != CreateObject(&format_id, &IID_IInArchive, reinterpret_cast<void**>(&in_archive))) {
        throw std::runtime_error("Failed to create archive object");
    }

    if (S_OK != in_archive->Open(_in_stream, &scan_size, open_callback)) {
        throw std::runtime_error("Failed to open archive");
    }
    return in_archive;
}

} // namespace smooth7zip
