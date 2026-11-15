const CONFIG = {
  repo: 'https://github.com/peepon95/cyberdeck-lilygo-workshop',
  zip: 'https://github.com/peepon95/cyberdeck-lilygo-workshop/archive/refs/heads/main.zip',
  codex: 'https://chatgpt.com/download/',
  claude: 'https://claude.ai/download',
  platformio: 'https://docs.platformio.org/en/latest/core/installation/methods/installer-script.html'
};

const saved = JSON.parse(localStorage.getItem('cyberDeckGuide') || '{}');
const state = { os: saved.os || '', ai: saved.ai || 'codex', step: Number.isInteger(saved.step) ? saved.step : 0 };
const save = () => localStorage.setItem('cyberDeckGuide', JSON.stringify(state));
const isMac = () => state.os === 'mac';
const osName = () => isMac() ? 'Mac' : 'Windows';
const shellName = () => isMac() ? 'Terminal' : 'PowerShell';

const command = (label, value) => `<div class="copy-card"><span>${label}</span><code>${value}</code><button data-copy="${encodeURIComponent(value)}">Copy</button></div>`;
const prompt = value => `<div class="prompt-card"><span>PASTE INTO ${state.ai === 'claude' ? 'CLAUDE CODE' : 'CODEX'}</span><p>${value}</p><button data-copy="${encodeURIComponent(value)}">Copy prompt</button></div>`;
const list = (...items) => `<ol>${items.map(item => `<li>${item}</li>`).join('')}</ol>`;

const steps = [
  {
    label: 'Connect internet',
    render: () => `<p class="step-number">STEP 1 · ${osName().toUpperCase()}</p><h2>Connect your laptop to your phone hotspot.</h2>${list('Turn on Personal Hotspot on iPhone, or Wi-Fi Hotspot on Android.', `On your ${osName()}, open Wi-Fi settings.`, 'Choose your phone hotspot and enter its password.', 'Open a webpage to check that internet works.', 'Keep your phone connected to power if possible.')}<div class="note"><b>Why?</b> Your coding assistant and first software downloads need internet. Later, the Cyber Deck dashboard works locally without internet.</div>`
  },
  {
    label: 'Choose AI tool',
    render: () => `<p class="step-number">STEP 2</p><h2>Choose your AI coding tool.</h2><p>You only need one. Pick the tool you can sign into successfully.</p><div class="tool-choice" id="aiChoice"><button data-value="codex"><b>Codex</b><span>OpenAI coding agent</span></button><button data-value="claude"><b>Claude Code</b><span>Anthropic coding agent</span></button></div><div id="toolSetup">${toolSetup()}</div>`
  },
  {
    label: 'Download starter',
    render: () => `<p class="step-number">STEP 3</p><h2>Download the Cyber Deck starter.</h2><div class="download-row"><a class="button primary" href="${CONFIG.zip}">Download Starter ZIP ↓</a><a class="button secondary" href="${CONFIG.repo}" target="_blank" rel="noreferrer">View the code ↗</a></div>${list(isMac() ? 'Open Downloads in Finder and double-click the ZIP.' : 'Open Downloads in File Explorer, right-click the ZIP and choose Extract All.', 'Open the extracted cyberdeck-lilygo-workshop-main folder.', 'Check that platformio.ini and the src folder are inside.')}`
  },
  {
    label: 'Open project',
    render: () => `<p class="step-number">STEP 4</p><h2>Open the folder in ${state.ai === 'claude' ? 'Claude Code' : 'Codex'}.</h2>${state.ai === 'claude' ? `${list(`Open ${shellName()}.`, 'Move into the extracted project folder.', 'Start Claude Code from that folder.')}${command(shellName(), isMac() ? 'cd ~/Downloads/cyberdeck-lilygo-workshop-main && claude' : 'cd $HOME\\Downloads\\cyberdeck-lilygo-workshop-main; claude')}` : list('Open the Codex app.', 'Choose Open Folder.', 'Select cyberdeck-lilygo-workshop-main from Downloads.', 'Start a new chat for this project.')} ${prompt("I'm completely new to coding. Inspect this Cyber Deck PlatformIO project and explain platformio.ini and src/main.cpp in simple language. Do not change anything yet.")}`
  },
  {
    label: 'Install builder',
    render: () => `<p class="step-number">STEP 5</p><h2>Install PlatformIO Core.</h2><p>PlatformIO turns the project into firmware and uploads it to the LILYGO.</p>${command(shellName(), isMac() ? 'python3 -m pip install -U platformio' : 'py -m pip install -U platformio')}${command(shellName(), 'pio --version')}${prompt("Check whether PlatformIO CLI is installed. If it is missing, help me install it using the official PlatformIO instructions for my laptop. Then run pio --version. Do not change the firmware yet.")}<p class="small-link"><a href="${CONFIG.platformio}" target="_blank" rel="noreferrer">Official PlatformIO installation guide ↗</a></p>`
  },
  {
    label: 'Connect LILYGO',
    render: () => `<p class="step-number">STEP 6</p><h2>Plug in your LILYGO.</h2>${list('Use a USB-C cable that supports data, not charging only.', 'Connect the LILYGO T-Display-S3 to the laptop.', isMac() ? 'Open Terminal and list the connected serial ports.' : 'Open Device Manager → Ports and find the new COM port.', 'Keep the port name—you will use it when uploading.')}${isMac() ? command('Terminal', 'ls /dev/cu.*') : `<div class="note"><b>Windows port example:</b> COM7. Yours may use a different number.</div>`}<details><summary>Board not detected?</summary><p>Try another data cable or USB port. For recovery, hold BOOT, tap RST, release BOOT, then check the port again.</p></details>`
  },
  {
    label: 'Build and flash',
    render: () => `<p class="step-number">STEP 7</p><h2>Build and flash the starter.</h2>${prompt(`Build this existing LILYGO T-Display-S3 project with PlatformIO. Preserve its display pins, Wi-Fi setup and button GPIOs. Fix compilation errors using the smallest safe changes. Ask me before uploading to my connected ${osName()} serial port.`)}${command(shellName(), 'pio run')}<div class="note"><b>After the build succeeds:</b> tell ${state.ai === 'claude' ? 'Claude Code' : 'Codex'} the detected port and approve the upload. Do not copy another participant's port.</div><details><summary>If upload cannot connect</summary><p>Hold BOOT, tap RST, release BOOT, then retry. Press RST once after a successful upload if the screen stays blank.</p></details>`
  },
  {
    label: 'Open dashboard',
    render: () => `<p class="step-number">STEP 8</p><h2>Open your Cyber Deck dashboard.</h2>${list('On your phone, open Wi-Fi settings.', 'Join the Cyberdeck-XXXX network shown on the LILYGO.', 'Enter the password cyberdeck.', 'If the phone says No Internet, stay connected.', 'Open a browser and visit http://192.168.4.1.', 'Choose a display, pet or game mode.')}<div class="credentials"><span>WI-FI</span><b>Cyberdeck-XXXX</b><span>PASSWORD</span><b>cyberdeck</b><span>PAGE</span><b>192.168.4.1</b></div><div class="note"><b>Finished!</b> Return your laptop to the phone hotspot whenever you need AI again. USB is used for flashing; Cyber Deck Wi-Fi is used for the local dashboard.</div>`
  }
];

function toolSetup() {
  return state.ai === 'claude'
    ? `<div class="tool-panel"><h3>Install Claude Code</h3><p>Download Claude, sign in, then verify the command-line tool.</p><a class="button secondary" href="${CONFIG.claude}" target="_blank" rel="noreferrer">Official Claude download ↗</a>${isMac() ? command('Terminal', 'curl -fsSL https://claude.ai/install.sh | bash') : command('PowerShell', 'irm https://claude.ai/install.ps1 | iex')}${command(shellName(), 'claude --version')}</div>`
    : `<div class="tool-panel"><h3>Install Codex</h3><p>Download the Codex desktop app, install it and sign in.</p><a class="button secondary" href="${CONFIG.codex}" target="_blank" rel="noreferrer">Official Codex download ↗</a></div>`;
}

const projects = [
  ['PG', 'PetGotchi', 'Create your own pet companion with feeding, sleep, happiness and different expressions.'],
  ['RR', 'Rocket Rush', 'Move a neon spaceship with two buttons while it automatically shoots aliens.'],
  ['SN', 'Pixel Snake', 'Guide a growing pixel snake, collect snacks and avoid hitting its tail.'],
  ['CS', 'Catch the Stars', 'Move a basket left and right to catch stars before they fall.'],
  ['PR', 'Pixel Racer', 'Steer a tiny car between obstacles and chase a high score.'],
  ['SB', 'Study Buddy', 'Add a Pomodoro timer, break reminders and encouraging messages.'],
  ['NB', 'Digital Name Badge', 'Show custom names, colours, pixel art and scrolling animation.'],
  ['??', 'Your Own Idea', 'Invent an original pet, useful tool, artwork or two-button game.']
];

const helps = [
  ['No serial port', 'Try another USB-C data cable and USB port.'],
  ['Build failed', 'Copy the full error into your coding assistant and ask for the smallest safe fix.'],
  ['Upload failed', 'Check the port, then try BOOT + RST recovery mode.'],
  ['Blank screen', 'Press RST once. If still blank, restore the starter firmware.'],
  ['Cyberdeck Wi-Fi missing', 'Keep the board powered, press RST and wait ten seconds.'],
  ['192.168.4.1 will not open', 'Stay on Cyberdeck Wi-Fi and type http://192.168.4.1 exactly.']
];

function setChoices() {
  document.querySelectorAll('#osChoice button').forEach(button => button.setAttribute('aria-pressed', button.dataset.value === state.os));
  document.querySelectorAll('#aiChoice button').forEach(button => button.setAttribute('aria-pressed', button.dataset.value === state.ai));
}

function renderStep() {
  document.querySelector('#workshop').hidden = !state.os;
  if (!state.os) return;
  document.querySelector('#stepContent').innerHTML = steps[state.step].render();
  document.querySelector('#stepDots').innerHTML = steps.map((step, index) => `<button class="${index === state.step ? 'active' : ''}" data-step="${index}"><span>${index + 1}</span>${step.label}</button>`).join('');
  document.querySelector('#previousStep').disabled = state.step === 0;
  document.querySelector('#nextStep').textContent = state.step === steps.length - 1 ? 'See project ideas ↓' : 'Next step →';
  document.querySelectorAll('#stepDots button').forEach(button => button.onclick = () => goTo(+button.dataset.step));
  document.querySelectorAll('[data-copy]').forEach(button => button.onclick = () => copy(decodeURIComponent(button.dataset.copy)));
  document.querySelectorAll('#aiChoice button').forEach(button => button.onclick = () => { state.ai = button.dataset.value; save(); renderStep(); });
  setChoices();
}

function goTo(index) {
  state.step = Math.max(0, Math.min(steps.length - 1, index)); save(); renderStep();
  document.querySelector('#workshop').scrollIntoView({ behavior: 'smooth', block: 'start' });
  document.querySelector('#stepContent').focus({ preventScroll: true });
}

async function copy(value) {
  try { await navigator.clipboard.writeText(value); }
  catch { const area = document.createElement('textarea'); area.value = value; document.body.append(area); area.select(); document.execCommand('copy'); area.remove(); }
  const toast = document.querySelector('#toast'); toast.textContent = 'Copied!'; toast.classList.add('show'); setTimeout(() => toast.classList.remove('show'), 1400);
}

document.querySelectorAll('#osChoice button').forEach(button => button.onclick = () => { state.os = button.dataset.value; state.step = 0; save(); setChoices(); renderStep(); document.querySelector('#workshop').scrollIntoView({ behavior: 'smooth' }); });
document.querySelector('#previousStep').onclick = () => goTo(state.step - 1);
document.querySelector('#nextStep').onclick = () => state.step === steps.length - 1 ? document.querySelector('#ideas').scrollIntoView({ behavior: 'smooth' }) : goTo(state.step + 1);
document.querySelector('#projectGrid').innerHTML = projects.map((project, index) => `<article><span>${project[0]}</span><h3>${project[1]}</h3><p>${project[2]}</p><button data-project="${index}">Copy starter prompt</button></article>`).join('');
document.querySelectorAll('[data-project]').forEach(button => button.onclick = () => copy(`Inspect my existing Cyber Deck LILYGO T-Display-S3 PlatformIO project first. Help me build ${projects[+button.dataset.project][1]}: ${projects[+button.dataset.project][2]} Preserve the verified 320x170 display configuration, Wi-Fi, two button GPIOs and existing working features. Build with PlatformIO, explain how to test it, and ask before flashing or publishing.`));
document.querySelector('#helpGrid').innerHTML = helps.map(item => `<details><summary>${item[0]}</summary><p>${item[1]}</p></details>`).join('');
setChoices(); renderStep();

if ('serviceWorker' in navigator && location.protocol.startsWith('http')) navigator.serviceWorker.register('./sw.js').catch(() => {});
