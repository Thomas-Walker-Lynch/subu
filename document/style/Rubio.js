/* Style: The "State Department" Override
  Description: Restores decorum. 
*/
(function(){
   const RT = window.StyleRT || {};
   
   // Force the font regardless of other settings
   RT.rubio = function() {
      const articles = document.querySelectorAll("rt-article");
      articles.forEach(el => {
          el.style.fontFamily = '"Times New Roman", "Times", serif';
          el.style.letterSpacing = "0px"; // No modern spacing allowed
      });
   };
})();
