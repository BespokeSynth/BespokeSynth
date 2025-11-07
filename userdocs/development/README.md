# Development

Guide for building BespokeSynth and contributing to the project.

## Overview

This section covers building BespokeSynth from source, understanding the architecture, and contributing to the project.

## Topics

### [Building](building.md)
Build BespokeSynth from source code.

**Platforms covered**:
- Windows (Visual Studio)
- macOS (Xcode)
- Linux (Make/CMake)

**Topics**:
- Prerequisites and dependencies
- Cloning the repository
- Building steps
- Troubleshooting build issues
- IDE setup

### [Architecture](architecture.md)
Understand BespokeSynth's architecture and design.

**Topics**:
- Module system
- Signal flow
- Event system
- Audio engine
- UI framework (JUCE)
- Python integration

### [Module Development](module-development.md)
Create custom modules in C++.

**Topics**:
- Module structure
- Creating a new module
- Module registration
- UI controls
- Signal processing
- Best practices

### [Contributing](contributing.md)
Contribute to the BespokeSynth project.

**Topics**:
- Code style guidelines
- Pull request process
- Testing requirements
- Documentation
- Community guidelines

### [Code Style](code-style.md)
Coding standards and conventions.

**Topics**:
- Naming conventions
- Formatting
- Comments and documentation
- Best practices
- Code review guidelines

## Quick Start

### Building from Source

**1. Clone Repository**:
```bash
git clone https://github.com/BespokeSynth/BespokeSynth.git
cd BespokeSynth
git submodule update --init --recursive
```

**2. Install Dependencies**:
- See [Building](building.md) for platform-specific instructions

**3. Build**:
- **Windows**: Open `BespokeSynth.sln` in Visual Studio
- **macOS**: Open `BespokeSynth.xcodeproj` in Xcode
- **Linux**: Run `make` in terminal

**4. Run**:
- Built executable in `build/` directory

## Architecture Overview

### Module System

**Modules are the core building blocks**:
- Each module is a C++ class
- Inherits from `IDrawableModule`
- Registered with `ModuleFactory`
- Self-contained functionality

**Module lifecycle**:
1. Creation (factory)
2. Initialization
3. Processing (audio/events)
4. Drawing (UI)
5. Destruction

### Signal Flow

**Four signal types**:
- **Notes**: MIDI-style messages
- **Audio**: Audio buffers
- **Pulses**: Trigger signals
- **Modulators**: Control voltage

**Processing**:
- Modules process in dependency order
- Audio runs in separate thread
- UI updates on main thread

### Event System

**Events drive behavior**:
- Note events
- Pulse events
- UI events
- MIDI events
- OSC events

**Event flow**:
1. Event generated (input, module, etc.)
2. Routed to connected modules
3. Processed by recipients
4. Results propagated

## Development Workflow

### Setting Up Development Environment

**1. IDE Setup**:
- Visual Studio 2019+ (Windows)
- Xcode 12+ (macOS)
- VSCode/CLion (Linux)

**2. Install Tools**:
- Git
- CMake (Linux)
- Python 3.x
- Audio drivers (ASIO, CoreAudio, JACK)

**3. Configure**:
- Set up debugger
- Configure build settings
- Install extensions/plugins

### Making Changes

**1. Create Branch**:
```bash
git checkout -b feature/my-feature
```

**2. Make Changes**:
- Edit code
- Follow code style
- Add comments

**3. Test**:
- Build successfully
- Test functionality
- Check for regressions

**4. Commit**:
```bash
git add .
git commit -m "Add feature: description"
```

**5. Push**:
```bash
git push origin feature/my-feature
```

**6. Create Pull Request**:
- Go to GitHub
- Create PR from your branch
- Describe changes
- Wait for review

## Creating a Module

### Basic Module Template

```cpp
#include "IDrawableModule.h"

class MyModule : public IDrawableModule
{
public:
    MyModule();
    virtual ~MyModule();
    
    // Setup
    void CreateUIControls() override;
    void Init() override;
    
    // Processing
    void Process(double time) override;
    
    // Drawing
    void DrawModule() override;
    
    // Serialization
    void LoadLayout(const ofxJSONElement& moduleInfo) override;
    void SaveLayout(ofxJSONElement& moduleInfo) override;
    
private:
    // Your controls and state
    float mValue;
    FloatSlider* mValueSlider;
};
```

### Registering Module

```cpp
// In ModuleFactory.cpp
REGISTER(MyModule, "mymodule", ModuleCategory::kModulatorModules);
```

### Adding Controls

```cpp
void MyModule::CreateUIControls()
{
    IDrawableModule::CreateUIControls();
    
    mValueSlider = new FloatSlider(this, "value", 5, 5, 100, 15, &mValue, 0, 1);
}
```

## Contributing Guidelines

### Code Style

**Naming**:
- Classes: `PascalCase`
- Functions: `PascalCase`
- Variables: `mCamelCase` (member), `camelCase` (local)
- Constants: `kCamelCase`

**Formatting**:
- Indent with 3 spaces
- Braces on same line
- Space after keywords

**Example**:
```cpp
class MyModule : public IDrawableModule
{
public:
   void ProcessAudio(double time)
   {
      for (int i = 0; i < mBufferSize; ++i)
      {
         mBuffer[i] *= mGain;
      }
   }
   
private:
   float mGain;
   int mBufferSize;
};
```

### Pull Request Process

**1. Before Submitting**:
- Code compiles without warnings
- Follows code style
- Includes tests if applicable
- Updates documentation

**2. PR Description**:
- Clear title
- Describe changes
- Reference issues
- Include screenshots if UI changes

**3. Review Process**:
- Maintainers review code
- Address feedback
- Make requested changes
- Resubmit for review

**4. Merging**:
- Approved by maintainer
- CI tests pass
- Merged to main branch

### Testing

**Manual Testing**:
- Test your changes thoroughly
- Test on target platform
- Check for regressions
- Test edge cases

**Automated Testing**:
- Unit tests (if applicable)
- Integration tests
- CI/CD pipeline

## Project Structure

```
BespokeSynth/
├── Source/               # C++ source code
│   ├── *.cpp            # Module implementations
│   ├── *.h              # Module headers
│   └── ModuleFactory.cpp # Module registration
├── resource/            # Resources
│   ├── fonts/          # Fonts
│   ├── images/         # Images
│   └── samples/        # Audio samples
├── libs/               # Third-party libraries
│   ├── JUCE/          # UI framework
│   ├── pybind11/      # Python bindings
│   └── ...
├── data/               # User data (runtime)
│   ├── savestate/     # Save files
│   ├── recordings/    # Audio recordings
│   └── ...
└── build/              # Build output
```

## Key Technologies

### JUCE Framework
- Cross-platform UI
- Audio I/O
- MIDI handling
- Plugin hosting

### Python Integration
- pybind11 for bindings
- Embedded interpreter
- Script module support

### Audio Processing
- Real-time audio thread
- Buffer-based processing
- Low-latency design

## Resources

### Documentation
- [JUCE Documentation](https://docs.juce.com/)
- [pybind11 Documentation](https://pybind11.readthedocs.io/)
- [C++ Reference](https://en.cppreference.com/)

### Community
- [GitHub Repository](https://github.com/BespokeSynth/BespokeSynth)
- [Discord Server](https://discord.gg/YdTMkvvpZZ)
- [Discussions](https://github.com/BespokeSynth/BespokeSynth/discussions)

### Tools
- [Visual Studio](https://visualstudio.microsoft.com/)
- [Xcode](https://developer.apple.com/xcode/)
- [CMake](https://cmake.org/)
- [Git](https://git-scm.com/)

## Getting Help

**Questions?**
- Ask on Discord
- Open GitHub discussion
- Check existing issues

**Found a Bug?**
- Search existing issues
- Create new issue with details
- Include steps to reproduce

**Want to Contribute?**
- Check open issues
- Ask on Discord
- Read contributing guidelines

## Next Steps

Dive deeper into development:

1. **[Building](building.md)** - Build from source
2. **[Architecture](architecture.md)** - Understand the system
3. **[Module Development](module-development.md)** - Create modules
4. **[Contributing](contributing.md)** - Contribute code
5. **[Code Style](code-style.md)** - Follow conventions

Or explore:
- **[Scripting](../scripting/README.md)** - Python scripting
- **[Advanced Topics](../advanced/README.md)** - Advanced features

