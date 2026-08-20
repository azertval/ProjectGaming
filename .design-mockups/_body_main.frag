<div class="screen" style="--accent: {{accent}}">
  <!-- ============ FOND : scene pixel art, bandes de ciel puis trois plans ============ -->
  <div style="position: absolute; inset: 0; background: #0d1220;"></div>
  <div style="position: absolute; left: 0; right: 0; top: 288px; height: 96px; background: #131b2d;"></div>
  <div style="position: absolute; left: 0; right: 0; top: 384px; height: 64px; background: #1a2338;"></div>
  <div style="position: absolute; left: 0; right: 0; top: 448px; height: 64px; background: #212c46;"></div>

  <!-- Etoiles : carres de 4 px, jamais de points antialiases. -->
  <div style="position: absolute; left: 96px; top: 64px; width: 4px; height: 4px; background: #f2f2ff;"></div>
  <div style="position: absolute; left: 288px; top: 128px; width: 4px; height: 4px; background: #8b93a7;"></div>
  <div style="position: absolute; left: 416px; top: 48px; width: 4px; height: 4px; background: #f2f2ff;"></div>
  <div style="position: absolute; left: 640px; top: 160px; width: 4px; height: 4px; background: #8b93a7;"></div>
  <div style="position: absolute; left: 736px; top: 80px; width: 4px; height: 4px; background: #f2f2ff;"></div>
  <div style="position: absolute; left: 960px; top: 208px; width: 4px; height: 4px; background: #8b93a7;"></div>
  <div style="position: absolute; left: 1088px; top: 112px; width: 4px; height: 4px; background: #f2f2ff;"></div>
  <div style="position: absolute; left: 1184px; top: 256px; width: 4px; height: 4px; background: #8b93a7;"></div>
  <div style="position: absolute; left: 544px; top: 240px; width: 4px; height: 4px; background: #8b93a7;"></div>

  <!-- Lune : cercle trace en pixels entiers, pas un border-radius. -->
  <div style="position: absolute; right: 128px; top: 72px;">
    <svg viewBox="0 0 16 16" width="128" height="128" shape-rendering="crispEdges" aria-hidden="true">
      <g fill="#e8ebf5">
        <rect x="5" y="0" width="6" height="1"></rect>
        <rect x="3" y="1" width="10" height="1"></rect>
        <rect x="2" y="2" width="12" height="1"></rect>
        <rect x="1" y="3" width="14" height="2"></rect>
        <rect x="0" y="5" width="16" height="6"></rect>
        <rect x="1" y="11" width="14" height="2"></rect>
        <rect x="2" y="13" width="12" height="1"></rect>
        <rect x="3" y="14" width="10" height="1"></rect>
        <rect x="5" y="15" width="6" height="1"></rect>
      </g>
      <g fill="#c3c9dc">
        <rect x="4" y="4" width="2" height="2"></rect>
        <rect x="9" y="7" width="3" height="2"></rect>
        <rect x="6" y="11" width="2" height="1"></rect>
      </g>
    </svg>
  </div>

  <!-- Plan lointain : silhouette industrielle a l'horizon. -->
  <div style="position: absolute; left: 0; right: 0; top: 416px; height: 96px; display: flex; align-items: flex-end; opacity: .55;">
    <div style="width: 96px; height: 40px; background: #2b3757;"></div>
    <div style="width: 32px; height: 96px; background: #2b3757;"></div>
    <div style="width: 64px; height: 56px; background: #2b3757;"></div>
    <div style="width: 128px; height: 32px; background: #2b3757;"></div>
    <div style="width: 32px; height: 88px; background: #2b3757;"></div>
    <div style="width: 32px; height: 72px; background: #2b3757;"></div>
    <div style="width: 160px; height: 40px; background: #2b3757;"></div>
    <div style="width: 96px; height: 64px; background: #2b3757;"></div>
    <div style="width: 32px; height: 96px; background: #2b3757;"></div>
    <div style="width: 224px; height: 48px; background: #2b3757;"></div>
    <div style="width: 64px; height: 80px; background: #2b3757;"></div>
    <div style="width: 128px; height: 32px; background: #2b3757;"></div>
    <div style="width: 32px; height: 64px; background: #2b3757;"></div>
    <div style="width: 160px; height: 40px; background: #2b3757;"></div>
  </div>

  <!-- Plan intermediaire : collines massives. -->
  <div style="position: absolute; left: 0; right: 0; top: 480px; height: 128px; display: flex; align-items: flex-end;">
    <div style="width: 256px; height: 64px; background: #1c2436;"></div>
    <div style="width: 192px; height: 96px; background: #1c2436;"></div>
    <div style="width: 320px; height: 48px; background: #1c2436;"></div>
    <div style="width: 224px; height: 112px; background: #1c2436;"></div>
    <div style="width: 288px; height: 72px; background: #1c2436;"></div>
  </div>

  <!-- Premier plan : sol et plateformes, au pas de 32 px du jeu. -->
  <div style="position: absolute; left: 0; right: 0; bottom: 0; height: 128px; background: #151b28; box-shadow: inset 0 8px 0 #232c40;"></div>
  <div style="position: absolute; left: 128px; bottom: 128px; width: 192px; height: 32px; background: #151b28; box-shadow: inset 0 8px 0 #232c40;"></div>
  <div style="position: absolute; left: 896px; bottom: 128px; width: 256px; height: 32px; background: #151b28; box-shadow: inset 0 8px 0 #232c40;"></div>
  <div style="position: absolute; left: 1024px; bottom: 224px; width: 128px; height: 32px; background: #151b28; box-shadow: inset 0 8px 0 #232c40;"></div>

  <!-- Voile de lisibilite : assombrit le fond sous le texte sans le cacher. -->
  <div style="position: absolute; left: 0; top: 0; bottom: 0; width: 704px; background: linear-gradient(to right, rgba(9,12,20,.88) 0%, rgba(9,12,20,.80) 55%, rgba(9,12,20,0) 100%);"></div>

  <!-- ============ CONTENU ============ -->
  <div style="position: absolute; inset: 0; display: flex; flex-direction: column; justify-content: center; padding: 0 96px;">
    <div class="title" style="font-size: 46px;">PROJECT<br>GAMING</div>

    <div style="height: 48px;"></div>

    <div style="display: flex; flex-direction: column; width: 520px;">
      <div class="row row--focus">@CARET@<span>Continuer</span></div>
      <div class="row">@CARET@<span>Nouvelle partie</span></div>
      <div class="row">@CARET@<span>Choisir un niveau</span></div>
      <div class="row">@CARET@<span>Mode Édition</span></div>
      <div class="row">@CARET@<span>Options</span></div>
      <div class="row">@CARET@<span>Crédits</span></div>
      <div class="row">@CARET@<span>Quitter</span></div>
    </div>
  </div>

  <div style="position: absolute; right: 40px; bottom: 32px; display: flex; align-items: center; gap: 20px;">
    <div class="hint"><span class="key">↑↓</span><span>Naviguer</span></div>
    <div class="hint"><span class="key">A</span><span>Valider</span></div>
    <div class="hint" style="color: var(--muted);">v0.1.0</div>
  </div>
</div>
