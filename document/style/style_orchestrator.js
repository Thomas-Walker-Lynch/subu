/*
  Master Loader & Orchestrator for StyleRT.
*/

window.StyleRT = window.StyleRT || {};

window.StyleRT.style_orchestrator = function() {
  const RT = window.StyleRT;
  
  const modules = [
    // Theme & Semantics
    'style/RT_title.js',       
    'style/theme_dark_gold.js',        
    'style/RT_term.js',          
    'style/RT_math.js',          
    'style/RT_code.js',          
    'style/article_tech_ref.js',
    'style/RT_TOC.js',            

    // Layout & Pagination
    'style/paginate_by_element.js', 
    'style/page_fixed_glow.js',             

    // Visibility
    'style/body_visibility_visible.js' 
  ];

  // 1. Bootloader
  const utility = document.createElement('script');
  utility.src = 'style/utility.js';
  
  utility.onload = () => { load_next(0); };
  utility.onerror = () => { console.error("StyleRT: Critical failure - utility.js missing."); };
  document.head.appendChild(utility);

  // 2. The Chain Loader
  const load_next = (index) => {
    if (index >= modules.length) {
      run_style();
      return;
    }
    const src = modules[index];
    if (RT.debug) RT.debug.log('style', `Loading: ${src}`);

    const script = document.createElement('script');
    script.src = src;
    script.onload = () => load_next(index + 1);
    script.onerror = () => { 
      console.error(`StyleRT: Failed load on ${src}`); 
      load_next(index + 1); 
    };
    document.head.appendChild(script);
  };

  // 3. Phase 1: Semantics
  const run_style = () => {
    RT.debug.log('style', 'Starting Phase 1: Setup & Semantics');

    // Naming Convention: RT.<filename_without_js>
    if(RT.theme) RT.theme();     
    if(RT.article) RT.article(); 
    
    // NEW: Trigger the Title Generator
    if(RT.RT_title) RT.RT_title(); 

    if(RT.RT_term) RT.RT_term();
    if(RT.RT_math) RT.RT_math();
    if(RT.RT_code) RT.RT_code();

    if (window.MathJax && MathJax.Hub && MathJax.Hub.Queue) {
      RT.debug.log('style', 'MathJax detected. Queueing layout tasks...');
      MathJax.Hub.Queue(["Typeset", MathJax.Hub], continue_style);
    } else {
      continue_style();
    }
  };

  // 4. Phase 2: Layout
  const continue_style = () => {
    RT.debug.log('style', 'Starting Phase 2: Layout & Reveal');
    
    // Debug: Dump the config to see what values we are using
    if(RT.debug) RT.debug.log('config', JSON.stringify(RT.config || {}, null, 2));
    
    if(RT.RT_TOC) RT.RT_TOC();
    if(RT.paginate_by_element) RT.paginate_by_element();
    if(RT.page) RT.page(); // Defined in page_css_pn.js
    if(RT.body_visibility_visible) RT.body_visibility_visible();
    
    RT.debug.log('style', 'Style execution complete.');
  };
};
