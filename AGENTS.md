# Clearwater Engine Development Guidelines

## Project direction

- Clearwater is a game engine written in C++20.
- Use a C-style approach within C++: prefer procedural APIs, plain data structures, free functions, explicit state, and straightforward control flow over heavy object-oriented abstractions.
- Defold is the closest architectural inspiration. Treat it as a reference for simplicity and engine organization, not as a requirement to copy its implementation.

## Portability and rendering

- The engine is intended to be cross-platform.
- The renderer must support multiple graphics APIs. OpenGL-specific types, assumptions, and behavior must remain inside the OpenGL backend.
- Keep the shared renderer interface backend-neutral so Vulkan and DirectX backends can be added later without redesigning engine-facing APIs.
- Do not shape generic rendering abstractions around OpenGL global state or `gl*` operations when a backend-independent resource, command, or state description is possible.

## Standard library and memory

- Minimize use of the C++ standard library.
- Memory ownership and lifetime must remain explicit and manually controlled by the engine.
- Do not introduce owning standard-library containers, smart pointers, or other abstractions that take over allocation and lifetime management by default.
- Standard-library facilities are acceptable for common, clearly justified cases and when they do not take ownership of dynamic memory. Examples include type traits, concepts, simple utility functions, algorithms, and non-owning views.
- Make allocation, ownership transfer, and destruction visible in APIs. Every owned resource must have a clear owner and destruction path.
- Avoid hidden allocations in frequently executed engine and rendering code.

## Templates and concepts

- C++20 templates and concepts are allowed when they make code reusable or constraints clearer.
- Keep template code simple, readable, and easy to debug.
- Avoid template magic, deeply nested metaprogramming, elaborate SFINAE, and abstractions whose behavior is difficult to understand from the call site.
- Prefer explicit code over a generic framework when the generic version adds substantial complexity for little practical benefit.
