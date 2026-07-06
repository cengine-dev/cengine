# Build & Testes — Ambiente Verificado

Registro da verificação de que a suíte de testes do CEngine builda e roda neste
ambiente. Serve como âncora para executar o [plano de melhoria](task/README.md)
com segurança (rodar `ctest` a cada tarefa).

- **Data da verificação:** 2026-07-04
- **Plataforma:** Windows 11 Pro
- **Resultado:** ✅ **28/28 testes passaram** (build limpo, sem warnings)

## Toolchain confirmada

| Ferramenta | Versão / Local |
|-----------|----------------|
| CMake | 4.0.2 (`C:\Program Files\CMake\bin\cmake.exe`) |
| Compilador | g++ 15.1.0 — MSYS2 UCRT64 (`C:\msys64\ucrt64\bin\g++.exe`) |
| Ninja | `C:\msys64\usr\bin\ninja.exe` |
| GoogleTest | v1.17.0 (via FetchContent, já em cache) |

> MSVC (`cl`) não está disponível no PATH neste ambiente; o build usa o preset
> MinGW/MSYS2.

## Como reproduzir

O working directory precisa ser a raiz do projeto **cengine**. Nesta máquina
(MSYS2 UCRT64) o preset que funciona é o `msys2-mingw`, que vive em
`CMakeUserPresets.json` (**não versionado** — ver tarefa 08). Os presets
portáveis versionados (`debug`/`release`/`asan`, geradores Ninja) são para
CI/outras máquinas; no MSYS2 o `ninja` embutido mastiga os paths nativos do
compilador, então prefira o preset local `msys2-mingw` aqui.

```powershell
# 1. Garantir o toolchain MSYS2 no PATH
$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"

# 2. Entrar no diretório do projeto
Set-Location C:\Users\mrmar\Documents\projetos_de_estudo\c++\cengine

# 3. Configurar (baixa GoogleTest na 1ª vez; precisa de rede)
cmake --preset msys2-mingw

# 4. Buildar a lib + o executável de testes
cmake --build --preset msys2-mingw

# 5. Rodar os testes
Set-Location out/build/msys2-mingw
ctest --output-on-failure
```

Alternativa (rodar o binário direto):

```powershell
out/build/msys2-mingw/tests/cengine_tests.exe
```

> **Estrutura (pós-tarefa 05a):** o projeto é multi-target —
> `cengine::core` (`core/`) + `cengine::routing` (`modules/routing/`) + testes
> (`tests/`, organizados por camada). Módulos são opt-in via
> `-DCENGINE_BUILD_ROUTING=ON/OFF` e `-DCENGINE_BUILD_TESTS=ON/OFF`. O core
> builda sozinho sem o routing.

## Saída obtida

```
-- Configuring done (1.5s)
-- Generating done (0.1s)
[100%] Built target cengine_tests
...
100% tests passed, 0 tests failed out of 28
Total Test time (real) = 0.20 sec
```

## Cobertura da suíte (28 testes)

- `SceneRepositoryTest` — registro/instanciação lazy via factory, unload,
  clone de estado, comparação de estados.
- `RouterInMemoryTest` — API enxuta do Router e delegação para o repositório.
- `GameManagerTest` — ciclo `onEnter/render/input/onExit` e condição de saída.
- `SceneLifetimeTest` (integração) — navegar → unload nas implementações reais,
  garantindo que nenhuma referência de cena sobrevive ao unload (rede contra
  *use-after-free*; ver [Tarefa 06](task/06-scene-lifetime.md)).
- `EngineManagerTest` + `EngineManagerIntegrationTest` — loop principal validado
  por **call-log** (ordem exata das chamadas).

## Observações ligadas ao plano de melhoria (resolvidas)

Snapshot de 2026-07-04, mantido como registro. Os três pontos já foram
endereçados pelo plano:

1. ✅ **Preset por sorte de ambiente** (caminhos hardcoded no `CMakePresets.json`)
   → resolvido na **[Tarefa 08](task/08-presets-and-ci.md)**: presets portáveis
   versionados + máquina-específicos no `CMakeUserPresets.json` (git-ignored).

2. ✅ **Rede de segurança** para a cirurgia do `IRouter` → o redesenho foi feito
   na **[Tarefa 05b](task/05b-redesign-irouter.md)** com a suíte verde.

3. ✅ **Nomes invertidos nos testes** (`ShouldExist…`, `IsNextState…Equals…`)
   → renomeados junto do código na **[Tarefa 02](task/02-fix-inverted-names.md)**.

## Protocolo para as tarefas do plano

A cada tarefa concluída: rebuildar e rodar `ctest`. Nenhuma tarefa é considerada
`done` com a suíte quebrada (ver critérios de aceite em cada arquivo de tarefa).
