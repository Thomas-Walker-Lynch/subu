/*
  Article Layout: Technical Reference
  Standard: Theme 1.0
  Description: High-readability layout for technical documentation on screens.
  Features: Sans-serif, justified text, distinct headers, boxed code.
*/
(function(){
  const RT = window.StyleRT = window.StyleRT || {};

  RT.article = function() {
    const debug = RT.debug || { log: function(){} };
    debug.log('layout', 'RT.article starting...');

    RT.config = RT.config || {};
    
    // Default Configuration
    RT.config.article = {
       font_family: '"Noto Sans", "Segoe UI", "Helvetica Neue", sans-serif'
      ,line_height: "1.8"       
      ,font_size: "16px"        
      ,font_weight: "400"       // Default (String)
      ,max_width: "820px" 
      ,margin: "0 auto"
    };

    // SAFE THEME DETECTION
    // If the theme is loaded and explicitly Light, bump the weight.
    try {
      if (RT.config.theme && RT.config.theme.meta_is_dark === false) {
         RT.config.article.font_weight = "600";
         debug.log('layout', 'Light theme detected: adjusting font weight to 600.');
      }
    } catch(e) {
      console.warn("StyleRT: Auto-weight adjustment failed, using default.", e);
    }

    const conf = RT.config.article;
    const article_seq = document.querySelectorAll("RT-article");

    if(article_seq.length === 0) {
      debug.log('layout', 'No <RT-article> elements found. Exiting.');
      return;
    }

    // 1. Apply Container Styles
    article_seq.forEach( (article) =>{
      const style = article.style;
      style.display = "block";
      style.fontFamily = conf.font_family;
      style.fontSize = conf.font_size;
      style.lineHeight = conf.line_height;
      style.fontWeight = conf.font_weight;
      style.maxWidth = conf.max_width;
      style.margin = conf.margin;
      style.padding = "0 20px";
      style.color = "var(--rt-content-main)";
    });

    // 2. Inject Child Typography
    const style_id = 'rt-article-typography';
    if (!document.getElementById(style_id)) {
      debug.log('layout', 'Injecting CSS typography rules.');
      const style_el = document.createElement('style');
      style_el.id = style_id;
      
      style_el.textContent = `
        /* --- HEADERS --- */
        rt-article h1 { 
          color: var(--rt-brand-primary);
          font-size: 2.0em; 
          font-weight: 500; 
          text-align: center;
          margin-top: 1.2em; 
          margin-bottom: 0.6em; 
          border-bottom: 2px solid var(--rt-border-default);
          padding-bottom: 0.3em;
          line-height: 1.2;
          letter-spacing: -0.02em;
        }

        rt-article h2 { 
          color: var(--rt-brand-secondary);
          font-size: 1.5em; 
          font-weight: 400; 
          text-align: center;
          margin-top: 1.0em; 
          margin-bottom: 0.5em; 
        }

        rt-article h2 + h3 {
           margin-top: -0.3em; 
           padding-top: 0;
        }

        rt-article h3 { 
          color: var(--rt-brand-tertiary);
          font-size: 1.4em; 
          font-weight: 400;
          margin-top: 1.0em; 
          margin-bottom: 0.5em;
        }
        
        /* --- DEEP LEVELS (H4-H6) --- */
        rt-article h4, rt-article h5, rt-article h6 {
           color: var(--rt-brand-tertiary);
           font-weight: bold;
           margin-top: 1.2em;
           font-style: italic;
        }
        rt-article h4 { margin-left: 2em; }
        rt-article h5 { margin-left: 4em; }
        rt-article h6 { margin-left: 6em; }

        /* --- BODY TEXT --- */
        rt-article p { 
          margin-bottom: 1.4em; 
          text-align: justify; 
          hyphens: auto;
          color: var(--rt-content-main);
        }

        /* --- RICH ELEMENTS --- */
        rt-article blockquote { 
          border-left: 4px solid var(--rt-brand-secondary); 
          margin: 1.5em 0; 
          padding: 0.5em 1em; 
          font-style: italic; 
          color: var(--rt-content-muted);
          background: var(--rt-surface-1);
          border-radius: 0 4px 4px 0;
        }

        rt-article ul, rt-article ol {
          margin-bottom: 1.4em;
          padding-left: 2em;
        }
        rt-article li {
           margin-bottom: 0.4em;
        }
        rt-article li::marker {
          color: var(--rt-brand-secondary);
          font-weight: bold;
        }
        
        /* Links */
        rt-article a {
          color: var(--rt-brand-link);
          text-decoration: none;
          border-bottom: 1px dotted var(--rt-border-default);
          transition: all 0.2s;
        }
        rt-article a:hover {
          color: var(--rt-brand-primary);
          border-bottom: 1px solid var(--rt-brand-primary);
          background: var(--rt-surface-1);
        }
        
        /* --- TECHNICAL --- */
        rt-article pre {
           background: var(--rt-surface-code);
           padding: 1em;
           border-radius: 4px;
           overflow-x: auto;
           border: 1px solid var(--rt-border-default);
        }
      `;
      document.head.appendChild(style_el);
    }
  };
})();
