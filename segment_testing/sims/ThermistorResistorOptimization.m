   c   l   e   a   r       a   l   l   ;   
   c   l   o   s   e       a   l   l   ;   
   
   b   e   t   a       =       3   3   8   0   ;   
   r   N   o   m       =       1   0   e   3   ;   
   t   N   o   m       =       2   5   ;       %   C   
   t   M   i   n       =       1   0   ;       %   C   
   t   M   a   x       =       1   0   0   ;       %   C   
   v   S   o   u   r   c   e       =       5   ;   
   
   s   y   m   s       s   t   a   t   i   c   R   e   s   i   s   t   a   n   c   e   
   
   r   M   a   x       =       N   T   C   R   e   s   i   s   t   a   n   c   e   C   a   l   c   (   b   e   t   a   ,       t   M   i   n   ,       r   N   o   m   ,       t   N   o   m   )   ;   
   r   M   i   n       =       N   T   C   R   e   s   i   s   t   a   n   c   e   C   a   l   c   (   b   e   t   a   ,       t   M   a   x   ,       r   N   o   m   ,       t   N   o   m   )   ;   
   
   v   M   a   x       =       v   S   o   u   r   c   e       *       (   r   M   a   x       /       (   r   M   a   x       +       s   t   a   t   i   c   R   e   s   i   s   t   a   n   c   e   )   )   ;   
   v   M   i   n       =       v   S   o   u   r   c   e       *       (   r   M   i   n       /       (   r   M   i   n       +       s   t   a   t   i   c   R   e   s   i   s   t   a   n   c   e   )   )   ;   
   
   d   e   l   t   a   V       =       v   M   a   x       -       v   M   i   n   ;   
   
   i   d   e   a   l   R   e   s   i   s   t   a   n   c   e   s       =       s   o   l   v   e   (   d   i   f   f   (   d   e   l   t   a   V   ,       s   t   a   t   i   c   R   e   s   i   s   t   a   n   c   e   )       =   =       0   )   ;   
   i   d   e   a   l   R   e   s   i   s   t   a   n   c   e       =       d   o   u   b   l   e   (   i   d   e   a   l   R   e   s   i   s   t   a   n   c   e   s   (   f   i   n   d   (   i   d   e   a   l   R   e   s   i   s   t   a   n   c   e   s       >       0   )   )   )   ;   
   
   v   M   a   x   I   d   e   a   l       =       v   S   o   u   r   c   e       *       (   r   M   a   x       /       (   r   M   a   x       +       i   d   e   a   l   R   e   s   i   s   t   a   n   c   e   )   )   ;   
   v   M   i   n   I   d   e   a   l       =       v   S   o   u   r   c   e       *       (   r   M   i   n       /       (   r   M   i   n       +       i   d   e   a   l   R   e   s   i   s   t   a   n   c   e   )   )   ;   
   d   e   l   t   a   V   I   d   e   a   l       =       v   M   a   x   I   d   e   a   l       -       v   M   i   n   I   d   e   a   l   ;   
   
   d   i   s   p   (   "   I   d   e   a   l       R   e   s   i   s   t   a   n   c   e   :       "       +       i   d   e   a   l   R   e   s   i   s   t   a   n   c   e   )   ;   
   d   i   s   p   (   "   R   e   s   u   l   t   i   n   g       v   M   a   x   :       "       +       v   M   a   x   I   d   e   a   l   )   ;   
   d   i   s   p   (   "   R   e   s   u   l   t   i   n   g       v   M   i   n   :       "       +       v   M   i   n   I   d   e   a   l   )   ;   
   d   i   s   p   (   "   R   e   s   u   l   t   i   n   g       R   a   n   g   e       o   f       V   o   l   t   a   g   e   :       "       +       d   e   l   t   a   V   I   d   e   a   l   )   ;   
   
   
   f   u   n   c   t   i   o   n       r   N   e   w       =       N   T   C   R   e   s   i   s   t   a   n   c   e   C   a   l   c   (   b   e   t   a   ,       t   e   m   p   ,       r   N   o   m   ,       t   N   o   m   )   
                   r   N   e   w       =       r   N   o   m       *       e   x   p   (   b   e   t   a       *       (   1   /   (   t   e   m   p       +       2   7   3   )       -       (   1   /   (   t   N   o   m       +       2   7   3   )   )   )   )   ;   
   e   n   d