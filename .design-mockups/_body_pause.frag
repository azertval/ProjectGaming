<div class="screen" style="--accent: {{accent}}">
  <!-- Scene figee derriere le recouvrement (assombrie, jamais floutee : le flou n'existe pas en pixel art). -->
  <div style="position: absolute; inset: 0; background: #1b2432;"></div>
  <div class="grid-bg"></div>
  <div style="position: absolute; left: 0; right: 0; bottom: 0; height: 160px; background: #2a3547;"></div>
  <div style="position: absolute; left: 160px; bottom: 160px; width: 192px; height: 32px; background: #2a3547;"></div>
  <div style="position: absolute; left: 448px; bottom: 256px; width: 128px; height: 32px; background: #2a3547;"></div>
  <div style="position: absolute; left: 864px; bottom: 160px; width: 256px; height: 32px; background: #2a3547;"></div>
  <div style="position: absolute; left: 480px; bottom: 288px; width: 32px; height: 48px; background: #6fb2ff;"></div>
  <div style="position: absolute; left: 928px; bottom: 192px; width: 32px; height: 32px; background: #ff5c5c;"></div>
  <!-- Voile sombre du recouvrement -->
  <div style="position: absolute; inset: 0; background: rgba(10,13,19,.72);"></div>

  <!-- Compteur d'images/s : element de HUD, dessine PAR-DESSUS le voile de pause
       (il reste lisible quand le jeu est suspendu). -->
  <div style="position: absolute; right: 32px; top: 28px; font-family: 'Press Start 2P', monospace; font-size: 14px; color: #7ee06a; text-shadow: 3px 3px 0 #0a0d13;">60 IPS</div>

  <div style="position: absolute; inset: 0; display: flex; align-items: center; justify-content: center;">
    <div class="frame frame--accent" style="width: 560px; padding: 44px 0 36px;">
      <div class="title" style="font-size: 28px; text-align: center;">PAUSE</div>
      <div style="height: 36px;"></div>
      <div style="display: flex; flex-direction: column;">
        <div class="row row--focus" style="justify-content: flex-start; padding-left: 56px;">@CARET@<span>Reprendre</span></div>
        <div class="row" style="padding-left: 56px;">@CARET@<span>Recommencer le niveau</span></div>
        <div class="row" style="padding-left: 56px;">@CARET@<span>Options</span></div>
        <div class="row" style="padding-left: 56px;">@CARET@<span>Quitter vers le menu</span></div>
      </div>
      <div style="height: 28px;"></div>
      <div style="display: flex; justify-content: center; gap: 24px;">
        <div class="hint"><span class="key">ÉCHAP</span><span>Reprendre</span></div>
        <div class="hint"><span class="key">B</span><span>Retour</span></div>
      </div>
    </div>
  </div>
</div>
