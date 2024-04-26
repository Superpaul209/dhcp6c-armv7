--- dhcp6s.c.orig	2017-02-28 19:06:15.000000000 +0000
+++ dhcp6s.c	2024-04-13 13:30:00.000000000 +0000
@@ -149,9 +149,6 @@
 static int ctldigestlen;
 static char *pid_file = DHCP6S_PIDFILE;
 
-static inline int get_val32 __P((char **, int *, u_int32_t *));
-static inline int get_val __P((char **, int *, void *, size_t));
-
 static void usage __P((void));
 static void server6_init __P((void));
 static void server6_mainloop __P((void));
@@ -643,49 +640,6 @@
 			(void)dhcp6_ctl_readcommand(&r);
 		}
 	}
-}
-
-static inline int
-get_val32(bpp, lenp, valp)
-	char **bpp;
-	int *lenp;
-	u_int32_t *valp;
-{
-	char *bp = *bpp;
-	int len = *lenp;
-	u_int32_t i32;
-
-	if (len < sizeof(*valp))
-		return (-1);
-
-	memcpy(&i32, bp, sizeof(i32));
-	*valp = ntohl(i32);
-
-	*bpp = bp + sizeof(*valp);
-	*lenp = len - sizeof(*valp);
-
-	return (0);
-}
-
-static inline int
-get_val(bpp, lenp, valp, vallen)
-	char **bpp;
-	int *lenp;
-	void *valp;
-	size_t vallen;
-{
-	char *bp = *bpp;
-	int len = *lenp;
-
-	if (len < vallen)
-		return (-1);
-
-	memcpy(valp, bp, vallen);
-
-	*bpp = bp + vallen;
-	*lenp = len - vallen;
-
-	return (0);
 }
 
 static int
