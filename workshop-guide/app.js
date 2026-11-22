const CONFIG = {
  repo: 'https://github.com/peepon95/cyberdeck-lilygo-workshop',
  zip: 'https://github.com/peepon95/cyberdeck-lilygo-workshop/archive/refs/heads/main.zip',
  codex: 'https://chatgpt.com/download/',
  claude: 'https://claude.ai/download',
  lovable: 'https://lovable.dev/',
  platformio: 'https://docs.platformio.org/en/latest/core/installation/methods/installer-script.html'
};

const saved = JSON.parse(localStorage.getItem('cyberDeckGuide') || '{}');
const state = { os: saved.os || '', ai: saved.ai || 'codex', step: Number.isInteger(saved.step) ? saved.step : 0 };
const save = () => localStorage.setItem('cyberDeckGuide', JSON.stringify(state));
const isMac = () => state.os === 'mac';
const osName = () => isMac() ? 'Mac' : 'Windows';
const shellName = () => isMac() ? 'Terminal' : 'PowerShell';

const command = (label, value) => `<div class="copy-card"><span>${label}</span><code>${value}</code><button data-copy="${encodeURIComponent(value)}">Copy</button></div>`;
const prompt = (value, target = state.ai === 'claude' ? 'CLAUDE CODE' : state.ai === 'lovable' ? 'CODEX OR CLAUDE CODE' : 'CODEX') => `<div class="prompt-card"><span>PASTE INTO ${target}</span><p>${value}</p><button data-copy="${encodeURIComponent(value)}">Copy prompt</button></div>`;
const list = (...items) => `<ol>${items.map(item => `<li>${item}</li>`).join('')}</ol>`;

const steps = [
  {
    label: 'Connect internet',
    render: () => `<p class="step-number">STEP 1 · ${osName().toUpperCase()}</p><h2>Connect your laptop to your phone hotspot.</h2>${list('Turn on Personal Hotspot on iPhone, or Wi-Fi Hotspot on Android.', `On your ${osName()}, open Wi-Fi settings.`, 'Choose your phone hotspot and enter its password.', 'Open a webpage to check that internet works.', 'Keep your phone connected to power if possible.')}<div class="note"><b>Why?</b> Your coding assistant and first software downloads need internet. Later, the Cyber Deck dashboard works locally without internet.</div>`
  },
  {
    label: 'Choose AI tool',
    render: () => `<p class="step-number">STEP 2</p><h2>Choose how you want to build.</h2><p>Codex and Claude Code can edit and flash firmware. Lovable is an optional visual-design companion for dashboards and interfaces.</p><div class="tool-choice" id="aiChoice"><button data-value="codex"><b>Codex</b><span>Firmware, build and flash</span></button><button data-value="claude"><b>Claude Code</b><span>Firmware, build and flash</span></button><button data-value="lovable"><b>Lovable</b><span>Interface design + coding-agent handoff</span></button></div><div id="toolSetup">${toolSetup()}</div>`
  },
  {
    label: 'Download starter',
    render: () => `<p class="step-number">STEP 3</p><h2>Download the Cyber Deck starter.</h2><div class="download-row"><a class="button primary" href="${CONFIG.zip}">Download Starter ZIP ↓</a><a class="button secondary" href="${CONFIG.repo}" target="_blank" rel="noreferrer">View the code ↗</a></div>${list(isMac() ? 'Open Downloads in Finder and double-click the ZIP.' : 'Open Downloads in File Explorer, right-click the ZIP and choose Extract All.', 'Open the extracted cyberdeck-lilygo-workshop-main folder.', 'Check that platformio.ini and the src folder are inside.')}`
  },
  {
    label: 'Open project',
    render: () => openProjectStep()
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
    render: () => `<p class="step-number">STEP 7</p><h2>Build and flash the starter.</h2>${prompt(`Build this existing LILYGO T-Display-S3 beginner starter with PlatformIO. Preserve its verified display pins, Wi-Fi setup and button GPIOs. Fix compilation errors using the smallest safe changes. Ask me before uploading to my connected ${osName()} serial port.`)}${command(shellName(), 'pio run')}<div class="note"><b>After upload:</b> the screen first shows <b>HELLO WORLD!</b> and <b>MY FIRST LILYGO</b>. After three seconds it changes to the Wi-Fi Setup screen. That confirms the display and firmware are working.</div><details><summary>If upload cannot connect</summary><p>Hold BOOT, tap RST, release BOOT, then retry. Press RST once after a successful upload if the screen stays blank.</p></details>`
  },
  {
    label: 'Open dashboard',
    render: () => `<p class="step-number">STEP 8</p><h2>Connect your Cyber Deck to Wi-Fi.</h2><img class="setup-photo" src="assets/wifi-setup-reference.png" alt="LILYGO screen showing its Wi-Fi Setup network and local address">${list('Read the unique LILYGO-SETUP name on the screen.', 'On your phone or laptop, open Wi-Fi settings.', 'Join that LILYGO-SETUP network using the password cyberdeck.', 'If it says No Internet, stay connected.', 'Open http://192.168.4.1 in a browser.', 'Enter your phone hotspot name and password, then tap Save and connect.', 'Watch the LILYGO change to Wi-Fi Connected and show its assigned IP address.')}<div class="credentials"><span>WI-FI</span><b>LILYGO-SETUP-XXXXXX</b><span>PASSWORD</span><b>cyberdeck</b><span>SETUP PAGE</span><b>192.168.4.1</b></div><div class="note"><b>Next time:</b> the saved network reconnects automatically. If it cannot connect, the LILYGO setup network remains available so you can try again.</div>`
  }
];

function toolSetup() {
  if (state.ai === 'lovable') {
    return `<div class="tool-panel"><span class="partner-chip">SUPPORTING WORKSHOP PARTNER</span><h3>Design with Lovable</h3><p>Use Lovable to create the phone dashboard or visual concept. You will still use Codex or Claude Code for ESP32 firmware, PlatformIO and USB flashing.</p><a class="button secondary" href="${CONFIG.lovable}" target="_blank" rel="noreferrer">Open Lovable ↗</a></div>`;
  }
  return state.ai === 'claude'
    ? `<div class="tool-panel"><h3>Install Claude Code</h3><p>Download Claude, sign in, then verify the command-line tool.</p><a class="button secondary" href="${CONFIG.claude}" target="_blank" rel="noreferrer">Official Claude download ↗</a>${isMac() ? command('Terminal', 'curl -fsSL https://claude.ai/install.sh | bash') : command('PowerShell', 'irm https://claude.ai/install.ps1 | iex')}${command(shellName(), 'claude --version')}</div>`
    : `<div class="tool-panel"><h3>Install Codex</h3><p>Download the Codex desktop app, install it and sign in.</p><a class="button secondary" href="${CONFIG.codex}" target="_blank" rel="noreferrer">Official Codex download ↗</a></div>`;
}

function openProjectStep() {
  if (state.ai === 'lovable') {
    const lovablePrompt = 'Design a mobile-first local dashboard for my LILYGO Cyber Deck. Use a playful retro pixel-art style, large touch-friendly controls and a responsive phone layout. It must be a visual prototype that can later be simplified into HTML, CSS and JavaScript hosted by an ESP32. Do not use a cloud database, login, external API or runtime CDN. Do not attempt to compile or flash hardware.';
    return `<p class="step-number">STEP 4</p><h2>Prototype the interface in Lovable.</h2>${list('Open Lovable and create a new project.', 'Copy the prompt below and generate the first interface.', 'Keep the design focused on one small pet, game or dashboard.', 'When the design is ready, open the downloaded Cyber Deck starter in Codex or Claude Code.', 'Ask that coding agent to recreate only the approved lightweight interface inside the ESP32 project.')} ${prompt(lovablePrompt, 'LOVABLE')}<div class="note"><b>Important:</b> Lovable designs the interface; Codex or Claude Code integrates it with the existing firmware and PlatformIO. Do not let two tools edit the same files at the same time.</div>`;
  }
  if (state.ai === 'claude') {
    return `<p class="step-number">STEP 4</p><h2>Open the folder in Claude Code.</h2>${list(`Open ${shellName()}.`, 'Move into the extracted project folder.', 'Start Claude Code from that folder.')}${command(shellName(), isMac() ? 'cd ~/Downloads/cyberdeck-lilygo-workshop-main && claude' : 'cd $HOME\\Downloads\\cyberdeck-lilygo-workshop-main; claude')}${prompt("I'm completely new to coding. Inspect this Cyber Deck PlatformIO project and explain platformio.ini and src/main.cpp in simple language. Do not change anything yet.")}`;
  }
  return `<p class="step-number">STEP 4</p><h2>Open the folder in Codex.</h2>${list('Open the Codex app.', 'Choose Open Folder.', 'Select cyberdeck-lilygo-workshop-main from Downloads.', 'Start a new chat for this project.')}${prompt("I'm completely new to coding. Inspect this Cyber Deck PlatformIO project and explain platformio.ini and src/main.cpp in simple language. Do not change anything yet.")}`;
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
  ['LILYGO setup Wi-Fi missing', 'Keep the board powered, press RST and wait ten seconds.'],
  ['192.168.4.1 will not open', 'Stay on LILYGO-SETUP Wi-Fi and type http://192.168.4.1 exactly.']
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
document.querySelector('#projectGrid').innerHTML = projects.map((project, index) => `<article><span>${project[0]}</span><h3>${project[1]}</h3><p>${project[2]}</p><button data-project="${index}">Copy project prompt</button></article>`).join('');
document.querySelectorAll('[data-project]').forEach(button => button.onclick = () => {
  const project = projects[+button.dataset.project];
  const text = state.ai === 'lovable'
    ? `Create a mobile-first visual prototype for ${project[1]}: ${project[2]} Use a retro pixel-art style and large touch controls. Keep it suitable for a 320x170 LILYGO Cyber Deck companion interface. Use no login, cloud database, external API or runtime CDN. This is an interface prototype only; do not attempt to compile or flash hardware.`
    : `Inspect my existing Cyber Deck LILYGO T-Display-S3 PlatformIO project first. Help me build ${project[1]}: ${project[2]} Preserve the verified 320x170 display configuration, Wi-Fi, two button GPIOs and existing working features. Build with PlatformIO, explain how to test it, and ask before flashing or publishing.`;
  copy(text);
});
document.querySelector('#helpGrid').innerHTML = helps.map(item => `<details><summary>${item[0]}</summary><p>${item[1]}</p></details>`).join('');
setChoices(); renderStep();

if ('serviceWorker' in navigator && location.protocol.startsWith('http')) navigator.serviceWorker.register('./sw.js').catch(() => {});
