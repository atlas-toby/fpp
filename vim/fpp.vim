" F++ Syntax Highlighting for Vim
if exists("b:current_syntax")
  finish
endif

syn keyword fppType NUMARA doğruMu dogruMu culuk cul
syn keyword fppBool aynen yok
syn keyword fppCond eğer eger yada_böyleyse yada_boyleyse değilse degilse
syn keyword fppLoop döngü_icerisine_sok_bilader_ne_yapalım dongu_icerisine_sok_bilader_ne_yapalim
syn keyword fppFunc yaz hak_team foto bak_fonksiyon sal dosyaGezginiRiskAltında dosyaGezginiRiskAltinda temizle yrenk dur_kardeşim dur_kardesim aradur uyari uyarı Renk

syn match fppComment "#.*$"
syn match fppComment "//.*$"
syn region fppString start='"' end='"'
syn region fppString start="'" end="'"
syn match fppNumber "\<\d\+\(\.\d\+\)\?\>"

hi def link fppType Type
hi def link fppBool Boolean
hi def link fppCond Conditional
hi def link fppLoop Repeat
hi def link fppFunc Function
hi def link fppComment Comment
hi def link fppString String
hi def link fppNumber Number

let b:current_syntax = "fpp"
