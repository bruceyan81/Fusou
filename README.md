# 扶桑

**扶桑**(ふそう) は、**Windows x64 のコンソール環境** 向けに開発している C++20 製のコンソールゲームです。

プレイヤーは「光」を利用して隠された床とはしごを発見し、敵を避けながら各エリアのゴールを灯し、最終地点を目指します。ゲームは東アジアの神話に登場する「扶桑神木」をテーマにしており、中国と日本の文化に共通するイメージを取り入れています。

---

## プロジェクトの状態

本プロジェクトでは、基本的なゲームループと主要な機能はすでに実装されています。

現在は、不具合修正やリファクタリングを進めている段階です。

---

## 動作環境

本プロジェクトはクロスプラットフォーム対応は目的としていません。

推奨環境：

- Windows x64
- C++20 に対応した MSVC ツールチェーン
- CMake 3.20 以上
- CMake で利用可能なビルドジェネレーター（Ninja または Visual Studio 生成器など）
- PowerShell または pwsh

ゲームを実行する場合は、必ず **Windows コンソール ホスト** を使用してください。
現在のバージョンでは Windows Terminal に対応しておらず、画面が崩れる場合があります。

この制限は実行時のみのものであり、ビルドには影響しません。

---

## ビルド方法

本プロジェクトは CMake と MSVC を使用してビルドします。

現在、以下の 2 種類のビルド種別があります。

- `Dev`
- `Shipping`

ビルド時に PowerShell スクリプトを実行し、パレット用 JSON から C++ ヘッダーを生成します。

コマンドラインでビルドする場合は、CMake が Windows x64 の MSVC ツールチェーンを検出できる環境で実行してください。

以下の例では Ninja ジェネレーターを使用します。

### Dev ビルド例

```powershell
cmake -S . -B out\build\dev -G Ninja -DCMAKE_BUILD_TYPE=Debug -DFUSOU_STAGE=Dev
cmake --build out\build\dev
```

### Shipping ビルド例

```powershell
cmake -S . -B out\build\shipping -G Ninja -DCMAKE_BUILD_TYPE=Release -DFUSOU_STAGE=Shipping
cmake --build out\build\shipping
```

ビルド完了後、CMake が生成したターゲット出力ディレクトリで `Fusou.exe` を実行してください。
実行時は Windows コンソール ホストを使用してください。

---

## 操作方法

プレイヤーはキーボードでキャラクターと光を操作し、ゲームを進めます。

### 基本操作

| 操作                | キー            |
| ----------------- | ------------- |
| 決定 | Enter キー      |
| ゲーム終了             | Esc キー        |
| プレイヤー移動           | W/A/S/D キー    |
| 光の ON / OFF       | F キー          |
| 光の移動              | ↑/↓/←/→ キー    |
| ゴールを灯す            | ゴールに近づいて E キー |
| Cutscene のスキップ    | 表示が出た後に任意のキー  |
| Game シーンから Intro へ戻る | I キーを 3 回連続で押す |

### Dev 用デバッグ機能

以下の機能は、Dev ビルド種別でのみ使用するデバッグ用の機能です。

| 操作                         | キー             |
| -------------------------- | -------------- |
| Died シーンから死亡前の状態へ復帰        | R キーを 3 回連続で押す |
| Game シーンから直前に灯したゴールの状態へ復帰 | P キーを 3 回連続で押す |

---

## プロジェクト構成

```text
Assets
  ゲームで使用する JSON アセット

FusouMapForge
  JSON アセット編集ツール

Resources
  Windows 用リソースファイルとアイコン

src/App
  アプリケーション起動、アセット読み込み、オブジェクト組み立て

src/Core
  ゲーム状態、ゲームルール、メインループ、表示用データ、ワールドモデル

src/Platform
  Windows コンソール環境での入力、描画、時刻取得のアダプター

third_party
  サードパーティライブラリ

tools
  ビルド時に使用する補助スクリプト
```

現在のディレクトリ構成では、ランタイムロジック、プラットフォーム適応、アセット、ツールを分けて管理しています。

---

## 技術的なポイント

- Core と Platform を分離し、Platform 実装を差し替えやすい構成
- 固定タイムステップのゲームループ
- Cell Buffer によるコンソール画面生成と差分描画
- データ駆動のゲームアセット
- 可視化アセット編集ツール

---

## FusouMapForge

**FusouMapForge** は、本プロジェクト用の JSON アセット編集ツールです。

ゲームが使用するタイルマップを可視化しながら作成し、ゲームが読み込める JSON アセットとして出力します。

このツールは、ハードコードでゲーム画面を管理することを避けるために作成しました。

---

## AI について

本プロジェクトでは、コードのリファクタリングと品質改善の補助として Codex CLI を使用しています。

意図を正確に伝えるため、AI Agents に関する指示文書は作者の母語で作成しています。

---

## License

The original source code, game assets, documentation, and tool code in this repository are proprietary and are not licensed for reuse.

This repository is published for portfolio and review purposes only. Unauthorized copying, modification, redistribution, or reuse of the original project content is not permitted.

Third-party libraries included under `third_party/` are not covered by the restriction above. They remain subject to their own license terms. See `THIRD_PARTY_NOTICES.md` for details.
