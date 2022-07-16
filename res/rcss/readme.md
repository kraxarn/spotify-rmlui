All stylesheets are recompiled, and nothing needs to be done to use them in the application.
However, if you want to modify the stylesheets, edit the respective scss-file, and recompile the 
stylesheet using sass:
```
sass style.scss --style compressed --no-source-map style.rcss
```
replacing "style.css" with the name of the modified stylesheet. rcss-files should never be modified 
directly, and are therefor minified using `--style compressed`.