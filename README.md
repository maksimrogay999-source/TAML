# TAML (Totally Accurate Mod Loader)

> ⚠️ **Warning: This project is under active development.**
> ⚠️ **Внимание: Проект находится в активной разработке.**
>
> **It is NOT a working product yet.** Many features are unimplemented, crashes are possible, and stability is not guaranteed.
> **Это не рабочий продукт.** Многие функции не реализованы, возможны краши и нестабильная работа.

TAML is an experimental mod loader for **Totally Accurate Battle Simulator (TABS)** on Android.
TAML — это экспериментальный модлоадер для **Totally Accurate Battle Simulator (TABS)** на Android.

The goal is to allow users to load and run custom mods (written in Lua) directly in the mobile version of the game, without needing a PC or complex external tools.
Цель проекта — дать возможность загружать и выполнять пользовательские моды (на Lua) прямо в мобильной версии игры, без необходимости использовать ПК или сложные внешние инструменты.

## 🚧 Current Status / Текущий статус

Only the **basic framework (MVP)** is ready at the moment:
На данный момент готов только **каркас (MVP)**:

- ✅ Native library (`libmodloader.so`) successfully loads into the game process.
- ✅ Нативная библиотека (`libmodloader.so`) успешно загружается в процесс игры.

- ✅ LuaJIT initializes and executes basic scripts.
- ✅ LuaJIT инициализируется и выполняет базовые скрипты.

- ✅ Java bridge (JNI) is scaffolded.
- ✅ Есть заготовка для Java-моста (JNI).

- ❌ Mod API (FFI, calling game functions) — **in development**.
- ❌ API для модов (FFI, вызовы игровых функций) — **в разработке**.

- ❌ Ready-to-use mods (content unlock, unit editing, etc.) — **planned**.
- ❌ Готовые моды (разблокировка контента, изменение юнитов и т.д.) — **в планах**.

- ❌ End-user installation and usage — **not possible yet**.
- ❌ Установка и использование обычным пользователем — **пока невозможна**.

## 🛠️ For Developers / Для разработчиков

If you want to look at the code or help with development:
Если ты хочешь посмотреть на код или помочь с разработкой:

- Build instructions are in the `Makefile` (requires Android NDK and LuaJIT).
- Сборка проекта описана в `Makefile` (требуется Android NDK и LuaJIT).

- To test, you need to manually inject `libmodloader.so` into the game's APK (via Smali injection).
- Для тестирования нужно вручную встроить `libmodloader.so` в APK игры (через Smali-инъекцию).

- There is no detailed documentation yet, but it will come as the project evolves.
- Подробной документации пока нет, но она появится по мере развития проекта.

## 📅 Roadmap / Планы

1. Implement FFI access to `libil2cpp.so`.
1. Реализовать FFI-доступ к `libil2cpp.so`.

2. Build the first working mod (unlocking hidden content).
2. Сделать первый рабочий мод (разблокировка скрытого контента).

3. Add convenient loading of Lua scripts from a folder.
3. Добавить удобную загрузку Lua-скриптов из папки.

4. Write documentation for modders.
4. Написать документацию для мододелов.

## 📜 License / Лицензия

Not chosen yet. Until development is complete, the code is not recommended for use in third-party projects.
Пока не выбрана. До завершения разработки код не рекомендуется использовать в сторонних проектах.

---

**Stay tuned. Once a working version is available, installation instructions will be added here.**
**Следи за обновлениями. Как только появится рабочая версия — здесь будет инструкция по установке.**