# 🚀 C++ Course Repository 

Репозиторий для курса C++ от Григорьева.  
⚠️ Одна из лабораторных требует работу с графикой (рекомендуется SDL).  
Здесь вы найдёте инструкции по быстрой настройке окружения в Docker-контейнере под Linux.

---

## 📦 Необходимые программы

1. **Docker Desktop**  
   [Скачать Docker Desktop](https://www.docker.com/products/docker-desktop/)

2. **Visual Studio Code**  
   [Скачать VSCode](https://code.visualstudio.com/download)

3. **X Server** (для графики):
   - **Windows**: [VcXsrv](https://sourceforge.net/projects/vcxsrv/)  
     *При установке выберите "Disable access control"*
   - **macOS**: [XQuartz](https://www.xquartz.org/)
   - **Linux**: Встроенный (ничего устанавливать не нужно)

4. **Git**  
   [Скачать Git](https://git-scm.com/downloads)  
   *Убедитесь, что Git добавлен в переменные окружения.*

---

## 🔌 Расширения для VSCode

Установите следующие расширения:
- [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
- [CMake Tools](https://marketplace.visualstudio.com/items?itemName=twxs.cmake)
- [Remote Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)
- [Docker](https://marketplace.visualstudio.com/items?itemName=ms-azuretools.vscode-docker)

---

## 🛠️ Настройка репозитория

1. Создайте папку проекта и откройте её в VSCode.
2. В терминале (Git Bash) выполните:
   ```bash
   git clone https://github.com/Ftp-Glich/Cpp-Cource.git
    ```
Откройте папку Cpp-Cource в VSCode.

🐳 Подготовка Docker-контейнера
Настройка X Server
Windows:

Запустите VcXsrv с настройками:

"Multiple windows" → "Display number: -1"

Выберите "Disable access control"

Разрешите VcXsrv в брандмауэре (путь к vcxsrv.exe).

macOS:
Запустите XQuartz и разрешите подключения.

Обновление IP-адреса
Узнайте свой IPv4 через ipconfig (Windows) или ifconfig (macOS/Linux).

Вставьте этот IP в файлы:

.devcontainer/devcontainer.json

.devcontainer/Dockerfile

.vscode/launch.json

🚀 Запуск контейнера
Нажмите F1 → выберите Dev Containers: Reopen In Container.

После загрузки в левом нижнем углу появится имя контейнера.

В терминале выполните:

```bash
cd /build
cmake ..
make
```
Собранный файл app появится в папке /build.

✅ Проверка работы
Перейдите во вкладку Run and Debug.

Выберите конфигурацию Debug SDL App → нажмите ▶️.

Если появилось синее окно — всё работает! 🎉

❓ Помощь
Пишите в Telegram: @ftpdog
Не забудьте приложить скриншот ошибки!
