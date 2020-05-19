// Copyright(c) 2019 - 2020, #Momo
// All rights reserved.
// 
// Redistributionand use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
// 
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditionsand the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditionsand the following disclaimer in the documentation
// and /or other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "FileManager.h"
#include "Utilities/Logger/Logger.h"
#include "Utilities/Profiler/Profiler.h"
#include "Utilities/Memory/Memory.h"

namespace MxEngine
{
    void FileModule::AddDirectory(const FilePath& directory)
    {
        namespace fs = std::filesystem;
        auto it = fs::recursive_directory_iterator(directory, fs::directory_options::skip_permission_denied);
        for (const auto& entry : it)
        {
            if (entry.is_regular_file())
            {
                FileModule::AddFile(entry.path());
            }
        }
    }

    const FilePath& FileModule::GetFilePath(StringId filename)
    {
        if (!FileModule::FileExists(filename))
        {
            static FilePath empty;
            return empty;
        }
        return manager->filetable[filename];
    }

    bool FileModule::FileExists(StringId filename)
    {
        return manager->filetable.find(filename) != manager->filetable.end();
    }

    void FileModule::AddFile(const FilePath& file)
    {
        auto filename = file.string(); // we need to transform Resources\path\to.something -> path/to.something
        std::replace_if(filename.begin(), filename.end(), [](char c) { return c == FilePath::preferred_separator; }, '/');
        filename.erase(filename.begin(), filename.begin() + manager->root.size() + 1);

        auto filehash = MakeStringId(filename);
        if (manager->filetable.find(filehash) != manager->filetable.end())
        {
            Logger::Instance().Warning("MxEngine::FileManager", 
                Format("hash of file \"{0}\" conflicts with other one in the project: {1}", filename, manager->filetable[filehash].string()));
        }
        else
        {
            Logger::Instance().Debug("MxEngine::FileManager", Format("file added to the project: {0}", filename));
        }
        manager->filetable.emplace(filehash, file);
    }

    void FileModule::Init(const FilePath& rootPath)
    {
        manager = Alloc<FileManagerImpl>();
        MAKE_SCOPE_TIMER("MxEngine::FileManager", "FileManager::Init()");
        MAKE_SCOPE_PROFILER("FileManager::Init()");

        manager->root = rootPath.string();
        FileModule::AddDirectory(rootPath);
    }

    void FileModule::Clone(FileManagerImpl* other)
    {
        manager = other;
    }

    FileManagerImpl* FileModule::GetImpl()
    {
        return manager;
    }
}